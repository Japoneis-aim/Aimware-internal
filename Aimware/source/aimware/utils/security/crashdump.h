#pragma once

namespace CrashCapture {
	// VEH: our-module / fail-fast first chance.
	// VCH: any unhandled (client.dll leave AVs too).
	// Documents\Aimware\crash.log
	void Install();
	void Uninstall();
}
