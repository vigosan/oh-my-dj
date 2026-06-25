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

static void test_platform_service_names_match_obs()
{
	std::printf("labels map to the exact OBS service names\n");
	CHECK(PlatformServiceName("Twitch") == "Twitch");
	CHECK(PlatformServiceName("Facebook Live") == "Facebook Live");
	// OBS calls it "YouTube - RTMPS", not "YouTube" — the remap that avoids a
	// silent migration failure.
	CHECK(PlatformServiceName("YouTube") == "YouTube - RTMPS");
	CHECK(PlatformServiceName("Custom").empty());
	CHECK(PlatformServiceName("Whatever").empty());
}

static void test_migrate_known_platform_by_name()
{
	std::printf("a known platform migrates to its OBS service with server=auto\n");
	const MigratedTarget t = MigrateLegacyTarget("Twitch", "rtmp://live.twitch.tv/app");
	CHECK(!t.isCustom);
	CHECK(t.service == "Twitch");
	CHECK(t.server == "auto");
}

static void test_migrate_remaps_youtube_label()
{
	std::printf("the legacy 'YouTube' label migrates to 'YouTube - RTMPS'\n");
	const MigratedTarget t = MigrateLegacyTarget("YouTube", "rtmp://a.rtmp.youtube.com/live2");
	CHECK(!t.isCustom);
	CHECK(t.service == "YouTube - RTMPS");
	CHECK(t.server == "auto");
}

static void test_migrate_by_url_when_label_renamed()
{
	std::printf("a renamed label still migrates if its URL matches a known preset\n");
	// User typed a custom label but kept Twitch's preset URL.
	const MigratedTarget t = MigrateLegacyTarget("My Twitch", "rtmp://live.twitch.tv/app");
	CHECK(!t.isCustom);
	CHECK(t.service == "Twitch");
}

static void test_migrate_custom_stays_custom()
{
	std::printf("custom / own-server targets stay custom\n");
	const MigratedTarget c = MigrateLegacyTarget("Custom", "");
	CHECK(c.isCustom);
	CHECK(c.service.empty());

	const MigratedTarget own = MigrateLegacyTarget("My Server", "rtmp://my-own.example/live");
	CHECK(own.isCustom);
	CHECK(own.service.empty());
}

int main()
{
	test_known_platforms_have_urls();
	test_custom_and_unknown_have_no_url();
	test_is_preset_url_guards_overwrite();
	test_presets_are_well_formed();
	test_platform_service_names_match_obs();
	test_migrate_known_platform_by_name();
	test_migrate_remaps_youtube_label();
	test_migrate_by_url_when_label_renamed();
	test_migrate_custom_stays_custom();

	if (g_failures == 0) {
		std::printf("\nAll preset tests passed.\n");
		return 0;
	}
	std::printf("\n%d check(s) failed.\n", g_failures);
	return 1;
}
