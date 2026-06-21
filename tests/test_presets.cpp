// Pure-logic tests for streaming platform presets. No Qt / no libobs.

#include <cstdio>

#include "stream-presets.hpp"

using namespace ohmydj;

static int g_failures = 0;

#define CHECK(expr)                                                          \
	do {                                                                 \
		if (!(expr)) {                                               \
			std::printf("  FAIL: %s (line %d)\n", #expr, __LINE__); \
			++g_failures;                                       \
		}                                                           \
	} while (0)

static void test_known_platforms_have_urls()
{
	std::printf("known platforms resolve to an RTMP url\n");
	CHECK(PresetUrl("Twitch") == "rtmp://live.twitch.tv/app");
	CHECK(PresetUrl("YouTube") == "rtmp://a.rtmp.youtube.com/live2");
	CHECK(!PresetUrl("Facebook Live").empty());
}

static void test_custom_and_unknown_have_no_url()
{
	std::printf("custom / unknown platforms have no preset url\n");
	CHECK(PresetUrl("Custom").empty());
	CHECK(PresetUrl("Something Else").empty());
}

static void test_is_preset_url_guards_overwrite()
{
	std::printf("IsPresetUrl decides when auto-fill may overwrite\n");
	CHECK(IsPresetUrl(""));                                  // empty => safe to fill
	CHECK(IsPresetUrl("rtmp://live.twitch.tv/app"));         // a known preset => safe to switch
	CHECK(!IsPresetUrl("rtmp://my-own-server.example/live")); // hand-typed => keep it
}

static void test_presets_are_well_formed()
{
	std::printf("preset list is non-empty and labelled\n");
	const auto &presets = StreamPresets();
	CHECK(presets.size() >= 2);
	for (const StreamPreset &preset : presets)
		CHECK(!preset.label.empty());
}

int main()
{
	test_known_platforms_have_urls();
	test_custom_and_unknown_have_no_url();
	test_is_preset_url_guards_overwrite();
	test_presets_are_well_formed();

	if (g_failures == 0) {
		std::printf("\nAll preset tests passed.\n");
		return 0;
	}
	std::printf("\n%d check(s) failed.\n", g_failures);
	return 1;
}
