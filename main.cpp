#include <Game/Core.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>

// Script Config
static std::string FishMethod = "NetSmall"; // NetSmall, Bait, Lure, Cage, Harpoon
static double TabOutChance = 0.45;
// Trackers
static Counter FishedCounter;
// Vars
static std::vector<std::string> SpotsNames = { "Fishing spot", "Rod Fishing spot" };
static std::string Action;
static std::vector<std::string> Tools;
static std::vector<std::string> Items;

// [0]: Lumbridge in front of Fishing Tutor
static std::vector<Tile> BlackListedTiles = { Tile(3246, 3157, 0) };

void Setup()
{
    ScriptInfo Info;
    Info.Name = "TPowerFisher";
    Info.Description = "Powerfishes, what else needs to be said";
    Info.Version = "0.01";
    Info.Category = "Fishing";
    Info.Author = "Twinki";
    Info.UID = Info.Name + Info.Version;
    Info.ForumPage = "https://forum.alpacabot.org";
    SetScriptInfo(Info);

    RequestArgument("FishMethod", "Fishing method to use, valid entries: NetSmall, Bait, Lure, Cage, Harpoon ");
    RequestArgument("TabOutChance", "Chance to tab out, 0.00 to 1.00");
}

void GetArgs()
{
    FishMethod = GetArgument("FishMethod");
    std::string TabOutChanceStr = GetArgument("TabOutChance");

    if (!FishMethod.empty())
        Debug::Info << "FishMethod set to " << FishMethod << std::endl;
    else
        FishMethod = "NetSmall";

    if (!TabOutChanceStr.empty())
    {
        try
        {
            TabOutChance = std::stod(TabOutChanceStr); // std::stod might throw an exception if the string is doesn't start with an int/or isn't a double
            Debug::Info << "TabOutChance set to " << TabOutChance << std::endl;
        }  catch (std::int32_t E)
        {
            Debug::Warning << "TabOutChance was set to something other than a double, make sure TabOutChance is set to 0.00-1.00" << std::endl;
            TabOutChance = 0.45;
        }
    } else TabOutChance = 0.45;
}

bool OnStart()
{
    SetLoopDelay(250); // How fast Loop is delayed after returning true in Loop()
    GetArgs(); // Load args

    if (FishMethod == "NetSmall")
    {
        Action = "Net";
        Tools = { "Small fishing net" };
        Items = { "Raw shrimps", "Raw anchovies" };
    } else if (FishMethod == "Bait")
    {
        Action = "Bait";
        Tools = { "Fishing rod", "Fishing bait" };
        Items = { "Raw sardine", "Raw herring", "Raw pike"  };
    } else if (FishMethod == "Lure")
    {
        Action = "Lure";
        Tools = { "Fly fishing rod", "Feather" };
        Items = { "Raw trout", "Raw salmon" };
    } else if (FishMethod == "Cage")
    {
        Action = "Cage";
        Tools = { "Lobster pot" };
        Items = { "Raw lobster" };
    } else if (FishMethod == "Harpoon")
    {
        Action = "Harpoon";
        Tools = { "Harpoon" };
        Items = { "Raw tuna", "Raw swordfish", "Raw shark" };
    } else
        FishMethod.clear();
    return !FishMethod.empty(); // if a Fish method is set, start the script
}

std::string FormatCommas(const std::int64_t& Int) // Will format the passed int and return a string with commas
{
    static bool Set = false;
    if (!Set)
    {
        Set = true;
        std::setlocale(LC_NUMERIC, "");
    }

    char Buffer[128];
    sprintf(Buffer, "%'d", Int);
    return std::string(Buffer);
}

std::int64_t ToHour(const std::int64_t& Start, const std::int64_t& Current)
{
    const std::int64_t TimeElapsed = GetScriptTimer().GetTimeElapsed();
    if (TimeElapsed <= 0) return 0;
    return (((float) (Current - Start) ) / ((float) TimeElapsed / 3600000.0f));
}

std::string FormatRunescapeGold(const std::int64_t& Gold) // Will format the given int into "Runescape" gold type abbreviations
{
    if (Gold <= 0) return "0";

    double Millions = (double) Gold / 1000000.0;
    double Thousands = (double) Gold / 1000.0;

    std::stringstream Stream;
    if (Millions >= 1.00)
        Stream << std::fixed << std::setprecision(1) << Millions << "M";
    else if (Thousands >= 1.00)
        Stream << std::fixed << std::setprecision(0) << Thousands << "K";
    else
        Stream << Gold;

    return Stream.str();
}

void Status()
{
    static std::int32_t StartingXP = 0; // Declared static here - will always be in memory, is only set on declaration
    static std::int32_t StartingLevel = 0;

    if (StartingXP == 0) StartingXP = Stats::GetExperience(Stats::FISHING);
    if (StartingLevel == 0) StartingLevel = Stats::GetCurrentLevel(Stats::FISHING);

    std::int32_t CurrentXP = Stats::GetExperience(Stats::FISHING);
    std::int32_t XPGained = (CurrentXP - StartingXP);
    std::int32_t XPPerHour = ToHour(StartingXP, CurrentXP);
    std::int32_t LevelsGained = (Stats::GetCurrentLevel(Stats::FISHING) - StartingLevel);

    std::string Title   = "------------------- TPowerFisher by Twinki -------------------";
    std::string Runtime = "--- Runtime: " + MillisToHumanLong(GetScriptTimer().GetTimeElapsed());
    std::string Fished  = "--- Fished: " + std::to_string(FishedCounter.GetIterations());
    std::string XP      = "--- XP Gained: " + FormatCommas(XPGained) + " [" + FormatRunescapeGold(XPPerHour) + " p/h]";
    std::string Level   = "--- Level: " + std::to_string(Stats::GetCurrentLevel(Stats::FISHING)) + " (+" + std::to_string(LevelsGained) + ")";
    std::string Break   = "--- Break Time: " + MillisToHumanShort(BreakHandler::GetBreakTimer().GetTimeElapsed());
    std::string Fatigue = "--- Fatigue: " + std::to_string(Profile::GetFatigue());
    std::string Footer  = "--------------------------------------------------------------";

    Debug::Clear(); // Clear the console
    Debug::Info << Title << std::endl;
    Debug::Info << Runtime << std::endl;
    Debug::Info << Fished << std::endl;
    Debug::Info << XP << std::endl;
    Debug::Info << Level << std::endl;
    Debug::Info << Break << std::endl;
    Debug::Info << Fatigue << std::endl;
    Debug::Info << Footer << std::endl;

    static const Point TitleP = Point(5, 20);
    static const Point RuntimeP = (TitleP + Point(0, 15)); // Increase each point Y by 15
    static const Point FishedP = RuntimeP + Point(0, 15);
    static const Point XPP = FishedP + Point(0, 15);
    static const Point LevelP = XPP + Point(0, 15);
    static const Point BreakP = LevelP + Point(0, 15);
    static const Point FatigueP = BreakP + Point(0, 15);
    static const Point FooterP = FatigueP + Point(0, 15);

    Paint::Clear(); // Clear the paint
    Paint::DrawString(Title, TitleP, 0, 255, 255, 255);
    Paint::DrawString(Runtime, RuntimeP, 0, 255, 255, 255);
    Paint::DrawString(Fished, FishedP, 0, 255, 255, 255);
    Paint::DrawString(XP, XPP, 0, 255, 255, 255);
    Paint::DrawString(Level, LevelP, 0, 255, 255, 255);
    Paint::DrawString(Break, BreakP, 0, 255, 255, 255);
    Paint::DrawString(Fatigue, FatigueP, 0, 255, 255, 255);
    Paint::SwapBuffer(); // Swap the paint buffer, so the screen "refreshes" the paint
}

bool CheckBlacklist(const Tile& T)
{
    for (auto& BLT : BlackListedTiles)
        if (T == BLT)
            return false;
    return true;
}

bool IsFishing()
{
    const auto Player = Players::GetLocal();
    return Player.GetAnimationID() != -1;
}

bool FishSpot(const std::string& A) // const std::string reference
{
    const auto Spots = NPCs::GetAll(SpotsNames);

    for (auto& S : Spots)
    {
        if (CheckBlacklist(S.GetTile()))
        {
            if (Inventory::IsItemSelected())
                S.Interact();

            if (S.Interact(A))
            {
                Timer IT;
                const auto P = Players::GetLocal();
                while (IT.GetTimeElapsed() < 10000)
                {
                    if (IsFishing()) return true;
                    Wait(NormalRandom(450, 450 * 0.10));
                }
            }
        }
    }
    return false;
}

bool DropFish()
{
    if (!Inventory::Open()) // Will only attempt to open the inventory if it isn't already open
        return false;

    FishedCounter.Increment(Inventory::Count(Items));

    bool ShiftClick = Settings::IsShiftClickToDropOn();
    if (ShiftClick && !IsKeyDown(KEY_SHIFT))
        Interact::DownKey(KEY_SHIFT);

    const auto Fish = Inventory::GetItems(Items);

    for (const auto& F : Fish)
    {
        if (Inventory::IsItemSelected())
            F.Interact();
        Inventory::Drop(F); // This will shift-click drop if available, it'll only press the SHIFT modifier if it isn't already down
    }

    if (ShiftClick && IsKeyDown(KEY_SHIFT))
        Interact::UpKey(KEY_SHIFT);

    return (!Inventory::ContainsAny(Items));
}

bool Loop()
{
    if (Mainscreen::IsLoggedIn())
    {
        BreakHandler::Break(false); // This will take breaks set by the player profile whenever needed

        Status();

        if (!Inventory::ContainsAny(Tools))
        {
            Debug::Fatal << "Tools aren't found in the Players Inventory, you need ";
            for (auto& T : Tools)
                Debug::Fatal << T << " ";
            Debug::Fatal << "in the Players Inventory, stopping" << std::endl;
            return false;
        }

        if (Inventory::IsFull() && Inventory::ContainsAny(Items))
        {
            Debug::Info << "Inventory Full, attempting to empty Inventory" << std::endl;
            DropFish();
            Debug::Info << "DropFish ended, about to Wait" << std::endl;
            Wait(NormalRandom(350, 350 * 0.10));
        }

        if (!IsFishing() && !Mainscreen::IsMoving()) // We aren't fishing, and we aren't actively walking to a new fish spot
        {
            Debug::Info << "Not Fishing, gaining focus and attempting to Fish" << std::endl;
            GainFocus();
            if (FishSpot(Action))
            {
                bool TabOut = UniformRandom() <= TabOutChance;
                bool MouseOff = TabOut && UniformRandom() <= 0.45;

                Debug::Info << "Successfully fished";
                if (TabOut)
                {
                    if (MouseOff)
                    {
                        Debug::Info << ", mousing off client";
                        Antiban::MouseOffClient(true);
                    }else
                    {
                        Debug::Info << ", alt-tabbing" << std::endl;
                        Antiban::LoseClientFocus();
                    }
                } else
                    Debug::Info << std::endl;

                std::int32_t Random = NormalRandom(2200, 2200 * 0.15);
                if (UniformRandom() <= 0.10) Random *= 2; // 10% chance to multiply the random wait by 2
                if (UniformRandom() <= 0.05) Random *= 4; // 5% chance to multiply the random wait by 4, even smaller chance that it multiplies by 2, then 4
                Wait(Random);
            }
        }

        Wait(NormalRandom(2200, 2200 * 0.20));
        return true; // Return true to continue looping

    } else
    {
        return Login::LoginPlayer(); // If LoginPlayer fails, this will return false to stop the script. LoginPlayer will cout if anything bad happened
    }
    return true; // Return true to continue looping
}

bool OnBreak()
{
    return false;
}

void OnEnd()
{

}