// loader.cpp — manual map injector para Aimware.dll
// Projeto: Application | x64 | Release | SubSystem: Console

#include <windows.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <filesystem>
#include <vector>

// ── typedefs internos ────────────────────────────────────────────────────────
using fnDllMain = BOOL(WINAPI*)(HINSTANCE, DWORD, LPVOID);

struct ManualMapData {
    LPVOID  image_base;
    HMODULE(WINAPI* fn_load_library_a)(LPCSTR);
    FARPROC(WINAPI* fn_get_proc_address)(HMODULE, LPCSTR);
    void   (WINAPI* fn_rtl_add_function_table)(PRUNTIME_FUNCTION, DWORD, DWORD64);
    BOOL    initialized;
    DWORD   last_error;
};

// ── shellcode executado no processo alvo ─────────────────────────────────────
// Resolve relocações, imports, exception table e chama DllMain.
// Precisa ser position-independent — sem globals, sem CRT, sem strings fora do struct.
static void __stdcall MapperShellcode(ManualMapData* data) {
    if (!data || !data->image_base) return;

    auto base = reinterpret_cast<BYTE*>(data->image_base);
    auto dos   = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    auto nt    = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
    auto opt   = &nt->OptionalHeader;

    // 1) relocações
    DWORD_PTR delta = reinterpret_cast<DWORD_PTR>(base) - opt->ImageBase;
    if (delta && opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) {
        auto reloc = reinterpret_cast<IMAGE_BASE_RELOCATION*>(
            base + opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);

        while (reloc->VirtualAddress) {
            DWORD count = (reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
            auto  entries = reinterpret_cast<WORD*>(reloc + 1);
            for (DWORD i = 0; i < count; i++) {
                if ((entries[i] >> 12) == IMAGE_REL_BASED_DIR64) {
                    auto patch = reinterpret_cast<DWORD_PTR*>(
                        base + reloc->VirtualAddress + (entries[i] & 0xFFF));
                    *patch += delta;
                }
            }
            reloc = reinterpret_cast<IMAGE_BASE_RELOCATION*>(
                reinterpret_cast<BYTE*>(reloc) + reloc->SizeOfBlock);
        }
    }

    // 2) imports
    if (opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) {
        auto desc = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(
            base + opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

        while (desc->Name) {
            auto  mod_name = reinterpret_cast<char*>(base + desc->Name);
            HMODULE mod    = data->fn_load_library_a(mod_name);

            auto thunk_ref  = reinterpret_cast<IMAGE_THUNK_DATA*>(
                base + (desc->OriginalFirstThunk ? desc->OriginalFirstThunk : desc->FirstThunk));
            auto func_ref   = reinterpret_cast<IMAGE_THUNK_DATA*>(
                base + desc->FirstThunk);

            for (; thunk_ref->u1.AddressOfData; ++thunk_ref, ++func_ref) {
                if (IMAGE_SNAP_BY_ORDINAL(thunk_ref->u1.Ordinal)) {
                    func_ref->u1.Function = reinterpret_cast<ULONG_PTR>(
                        data->fn_get_proc_address(mod,
                            reinterpret_cast<LPCSTR>(IMAGE_ORDINAL(thunk_ref->u1.Ordinal))));
                } else {
                    auto ibn = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(
                        base + thunk_ref->u1.AddressOfData);
                    func_ref->u1.Function = reinterpret_cast<ULONG_PTR>(
                        data->fn_get_proc_address(mod, ibn->Name));
                }
            }
            ++desc;
        }
    }

    // 3) exception table (SEH x64 — necessário para safetyhook funcionar)
    if (opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size) {
        auto rf = reinterpret_cast<PRUNTIME_FUNCTION>(
            base + opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress);
        DWORD count = opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size
                      / sizeof(RUNTIME_FUNCTION);
        data->fn_rtl_add_function_table(rf, count,
            reinterpret_cast<DWORD64>(base));
    }

    // 4) DllMain
    if (opt->AddressOfEntryPoint) {
        auto entry = reinterpret_cast<fnDllMain>(base + opt->AddressOfEntryPoint);
        data->initialized = entry(
            reinterpret_cast<HINSTANCE>(base),
            DLL_PROCESS_ATTACH, nullptr);
    } else {
        data->initialized = TRUE;
    }
}

// marcador de fim do shellcode — precisa ficar imediatamente depois
static void ShellcodeEnd() {}

// ── helpers ──────────────────────────────────────────────────────────────────
static DWORD FindProcessId(const wchar_t* name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32W pe{ sizeof(pe) };
    DWORD pid = 0;
    if (Process32FirstW(snap, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, name) == 0) {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return pid;
}

static bool ReadDll(const std::filesystem::path& path, std::vector<BYTE>& out) {
    HANDLE f = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ,
                           nullptr, OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return false;

    LARGE_INTEGER sz{};
    GetFileSizeEx(f, &sz);
    out.resize(static_cast<size_t>(sz.QuadPart));

    DWORD read = 0;
    bool ok = ReadFile(f, out.data(), static_cast<DWORD>(out.size()), &read, nullptr)
              && read == out.size();
    CloseHandle(f);
    return ok;
}

// ── injetor principal ─────────────────────────────────────────────────────────
static bool ManualMap(HANDLE proc, const std::vector<BYTE>& raw) {
    auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(raw.data());
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
        printf("[!] header DOS invalido\n");
        return false;
    }
    auto nt  = reinterpret_cast<const IMAGE_NT_HEADERS*>(raw.data() + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) {
        printf("[!] header NT invalido\n");
        return false;
    }
    auto& opt = nt->OptionalHeader;

    // aloca imagem no processo alvo
    auto remote_image = reinterpret_cast<BYTE*>(
        VirtualAllocEx(proc, nullptr, opt.SizeOfImage,
                       MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (!remote_image) {
        printf("[!] VirtualAllocEx (imagem) falhou: %lu\n", GetLastError());
        return false;
    }
    printf("[+] imagem alocada em 0x%p (%u bytes)\n", remote_image, opt.SizeOfImage);

    // copia header
    WriteProcessMemory(proc, remote_image, raw.data(), opt.SizeOfHeaders, nullptr);

    // copia secoes
    auto sec = IMAGE_FIRST_SECTION(nt);
    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++, sec++) {
        if (!sec->SizeOfRawData) continue;
        WriteProcessMemory(proc,
            remote_image + sec->VirtualAddress,
            raw.data() + sec->PointerToRawData,
            sec->SizeOfRawData, nullptr);
    }
    printf("[+] %hu secoes copiadas\n", nt->FileHeader.NumberOfSections);

    // prepara ManualMapData no processo alvo
    HMODULE k32    = GetModuleHandleA("kernel32.dll");
    HMODULE nt_dll = GetModuleHandleA("ntdll.dll");

    ManualMapData data{};
    data.image_base           = remote_image;
    data.fn_load_library_a    = reinterpret_cast<decltype(data.fn_load_library_a)>(
                                    GetProcAddress(k32, "LoadLibraryA"));
    data.fn_get_proc_address  = reinterpret_cast<decltype(data.fn_get_proc_address)>(
                                    GetProcAddress(k32, "GetProcAddress"));
    data.fn_rtl_add_function_table = reinterpret_cast<decltype(data.fn_rtl_add_function_table)>(
                                    GetProcAddress(nt_dll, "RtlAddFunctionTable"));

    SIZE_T shellcode_size = reinterpret_cast<BYTE*>(ShellcodeEnd)
                          - reinterpret_cast<BYTE*>(MapperShellcode);

    // aloca shellcode + data no alvo
    auto remote_mem = reinterpret_cast<BYTE*>(
        VirtualAllocEx(proc, nullptr,
                       shellcode_size + sizeof(ManualMapData),
                       MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (!remote_mem) {
        printf("[!] VirtualAllocEx (shellcode) falhou: %lu\n", GetLastError());
        return false;
    }

    BYTE* remote_data = remote_mem + shellcode_size;
    WriteProcessMemory(proc, remote_mem,  reinterpret_cast<void*>(MapperShellcode), shellcode_size, nullptr);
    WriteProcessMemory(proc, remote_data, &data, sizeof(data), nullptr);

    printf("[+] shellcode em 0x%p, data em 0x%p\n", remote_mem, remote_data);

    // cria thread remota que executa o shellcode
    HANDLE thread = CreateRemoteThread(proc, nullptr, 0,
        reinterpret_cast<LPTHREAD_START_ROUTINE>(remote_mem),
        remote_data, 0, nullptr);
    if (!thread) {
        printf("[!] CreateRemoteThread falhou: %lu\n", GetLastError());
        return false;
    }

    printf("[*] aguardando DllMain...\n");
    WaitForSingleObject(thread, 8000);
    CloseHandle(thread);

    // verifica se inicializou
    ManualMapData result{};
    ReadProcessMemory(proc, remote_data, &result, sizeof(result), nullptr);

    // limpa shellcode/data da memoria (mantém imagem — dll precisa dela)
    VirtualFreeEx(proc, remote_mem, 0, MEM_RELEASE);

    if (!result.initialized) {
        printf("[!] DllMain retornou FALSE (error %lu)\n", result.last_error);
        return false;
    }

    printf("[+] injetado com sucesso\n");
    return true;
}

// ── entry point ───────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    printf("Aimware Loader v1.0\n");
    printf("----------------------\n");

    // dll: mesmo diretorio do exe, ou passada como argumento
    std::filesystem::path dll_path;
    if (argc >= 2) {
        dll_path = argv[1];
    } else {
        wchar_t exe_path[MAX_PATH]{};
        GetModuleFileNameW(nullptr, exe_path, MAX_PATH);
        dll_path = std::filesystem::path(exe_path).parent_path() / L"Aimware.dll";
    }

    if (!std::filesystem::exists(dll_path)) {
        printf("[!] dll nao encontrada: %ls\n", dll_path.c_str());
        printf("    coloca a dll no mesmo diretorio do loader ou passa o caminho como argumento\n");
        printf("\nEnter para sair...\n");
        getchar();
        return 1;
    }
    printf("[*] dll: %ls\n", dll_path.c_str());

    // aguarda cs2 abrir (ate 60s)
    printf("[*] aguardando cs2.exe...\n");
    DWORD pid = 0;
    for (int i = 0; i < 60 && !pid; i++) {
        pid = FindProcessId(L"cs2.exe");
        if (!pid) Sleep(1000);
    }
    if (!pid) {
        printf("[!] cs2.exe nao encontrado apos 60s\n");
        printf("\nEnter para sair...\n");
        getchar();
        return 1;
    }
    printf("[+] cs2.exe encontrado (PID %lu)\n", pid);

    // le dll do disco
    std::vector<BYTE> raw;
    if (!ReadDll(dll_path, raw)) {
        printf("[!] falha ao ler dll: %lu\n", GetLastError());
        printf("\nEnter para sair...\n");
        getchar();
        return 1;
    }
    printf("[+] dll lida (%zu bytes)\n", raw.size());

    // abre o processo com permissao de injecao
    HANDLE proc = OpenProcess(
        PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE |
        PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION,
        FALSE, pid);
    if (!proc) {
        printf("[!] OpenProcess falhou: %lu (rode como administrador)\n", GetLastError());
        printf("\nEnter para sair...\n");
        getchar();
        return 1;
    }

    bool ok = ManualMap(proc, raw);
    CloseHandle(proc);

    if (ok) {
        printf("\n[OK] Aimware carregado. Bom jogo.\n");
    } else {
        printf("\n[FAIL] injecao falhou.\n");
    }

    printf("\nEnter para sair...\n");
    getchar();
    return ok ? 0 : 1;
}
