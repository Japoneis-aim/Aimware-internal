#include "logo_texture.h"
#include "logo_png.hpp"

#include <Windows.h>
#include <d3d11.h>
#include <cstring>

// stb_image — only define implementation once per TU.
// weapon_icon_draw.cpp already defines STB_IMAGE_IMPLEMENTATION,
// so we just use the header here (no second define needed).
#include "../../../external/stb_image.h"

namespace LogoTexture {
namespace {

ID3D11ShaderResourceView* g_srv       = nullptr;
ID3D11Device*             g_srvDevice = nullptr;
bool                      g_tried     = false;

bool CreateTexture(ID3D11Device* device) {
    if (!device)
        return false;

    int w = 0, h = 0, ch = 0;
    unsigned char* rgba = stbi_load_from_memory(
        logo_png::kData,
        static_cast<int>(logo_png::kSize),
        &w, &h, &ch, 4);
    if (!rgba || w <= 0 || h <= 0) {
        if (rgba) stbi_image_free(rgba);
        return false;
    }

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width            = static_cast<UINT>(w);
    desc.Height           = static_cast<UINT>(h);
    desc.MipLevels        = 1;
    desc.ArraySize        = 1;
    desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage            = D3D11_USAGE_DEFAULT;
    desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA sub{};
    sub.pSysMem     = rgba;
    sub.SysMemPitch = static_cast<UINT>(w * 4);

    ID3D11Texture2D* tex = nullptr;
    HRESULT hr = device->CreateTexture2D(&desc, &sub, &tex);
    stbi_image_free(rgba);
    if (FAILED(hr) || !tex)
        return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format                    = desc.Format;
    srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels       = 1;

    ID3D11ShaderResourceView* srv = nullptr;
    hr = device->CreateShaderResourceView(tex, &srvDesc, &srv);
    tex->Release();
    if (FAILED(hr) || !srv)
        return false;

    g_srv       = srv;
    g_srvDevice = device;
    return true;
}

} // namespace

void EnsureReady(ID3D11Device* device) {
    if (!device)
        return;
    // Device changed (TDR / driver reset): drop old SRV and rebuild.
    if (g_srv && g_srvDevice != device) {
        g_srv->Release();
        g_srv       = nullptr;
        g_srvDevice = nullptr;
        g_tried     = false;
    }
    if (g_srv || g_tried)
        return;
    g_tried = true;
    CreateTexture(device);
}

ID3D11ShaderResourceView* GetSRV() {
    return g_srv;
}

ImVec2 GetSize() {
    return ImVec2(static_cast<float>(logo_png::kWidth),
                  static_cast<float>(logo_png::kHeight));
}

} // namespace LogoTexture
