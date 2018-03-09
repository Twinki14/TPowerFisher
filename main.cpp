#include <Game/Core.hpp>
#include <Core/Internal.hpp>
#include <Game/Tools/BreakHandler.hpp>
#include <Game/Tools/RandomHandler.hpp>

#include <iostream>
#include <vector>
#include <cstdarg>
#include <string>

// Script Config
static std::string FishMethod = "NetSmall"; // NetSmall, Bait, Lure, Cage, Harpoon
static double TabOutChance = 0.60;

// Trackers
static std::int32_t StartingXP = 0;
static std::int32_t StartingLevel = 0;
static std::uint32_t XPGained = 0;
static std::uint32_t LevelsGained = 0;
static Counter Fished;
static Countdown LoopCD;
static Countdown PrintCD(30000);
// Vars
static std::vector<std::string> SpotsNames = { "Fishing spot", "Rod Fishing spot" };
static std::string Action;
static std::vector<std::string> Tools;
static std::vector<std::string> Items;

// [0]: Lumbridge in front of Fishing Tutor
static std::vector<Tile> BlackListedTiles = { Tile(3246, 3157, 0) };

static std::int32_t SHIFT_CLICK_DROP_SETTING = 1055;
static std::int32_t SHIFT_CLICK_DROP_BIT_POS = 17;

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
    Debug::ShowConsole();
    Debug::SetVerbose(true);
    RandomHandler::SetCheckDelay(5000);
}

bool OnStart()
{
    FishMethod = GetArgument("FishMethod");
    if (FishMethod != "")
    {
        Debug::Info << "FishMethod set to " << FishMethod << std::endl;
    } else
    {
        FishMethod = "NetSmall";
        Debug::Warning << "FishMethod not set! Defaulting to " << FishMethod << std::endl;
    }

    std::string TabOutChanceS = GetArgument("TabOutChance");
    if (TabOutChanceS != "")
    {
        TabOutChance = std::stod(TabOutChanceS);
        Debug::Info << "TabOutChance set to " << TabOutChance << std::endl;
    }
    else
        Debug::Warning << "TabOutChance not set! Defaulting to " << TabOutChance << std::endl;

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
    return (FishMethod != "");
}

void PrintStatus()
{
    if (StartingXP == 0)
        StartingXP = Stats::GetExperience(Stats::FISHING);
    if (StartingLevel == 0)
        StartingLevel = Stats::GetCurrentLevel(Stats::FISHING);
    XPGained = (Stats::GetExperience(Stats::FISHING) - StartingXP);
    LevelsGained = (Stats::GetCurrentLevel(Stats::FISHING) - StartingLevel);
    Debug::Info << "------------------- TPowerFisher by Twinki -------------------" << std::endl;
    Debug::Info << "--- Runtime: " << MillisToHumanLong(GetScriptTimer().GetTimeElapsed()) << std::endl;
    Debug::Info << "--- Fished: " << Fished.GetIterations() << std::endl;
    Debug::Info << "--- XP Gained: " << XPGained << std::endl;
    Debug::Info << "--- Level: " << Stats::GetCurrentLevel(Stats::FISHING) << "(+" << LevelsGained << ")" << std::endl;
    Debug::Info << "--- Break Time: " << MillisToHumanShort(BreakHandler::GetBreakTimer().GetTimeElapsed()) << std::endl;
    Debug::Info << "--- Fatigue: " << Profile::GetFatigue() << std::endl;
    Debug::Info << "--------------------------------------------------------------" << std::endl;
    PrintCD.Reset();
}

bool CheckBlacklist(const Tile& T)
{
    for (auto& BLT : BlackListedTiles)
        if (T == BLT)
            return false;
    return true;
}

bool FishSpot(std::string& Action)
{
    std::vector<NPC> Spots = NPCs::GetAll(SpotsNames);

    for (auto& S : Spots)
    {
        Tile ST = NPCs::GetTileOf(S);
        if (CheckBlacklist(ST))
        {
            Convex C = NPCs::GetConvexOf(S);
            Paint::Clear();
            Paint::DrawConvex(C, 255, 0, 0, 255);

            if (Inventory::IsItemSelected())
                Interact::Click(S);

            if (Interact::Click(S, Action))
            {
                Timer IT;
                Player P = Players::GetLocal();
                while (IT.GetTimeElapsed() < 10000)
                {
                    Paint::Clear();
                    C = NPCs::GetConvexOf(S);
                    Paint::DrawConvex(C, 0, 255, 0, 255);
                    if (P.GetAnimationID() != -1)
                    {
                        Paint::Clear();
                        C = NPCs::GetConvexOf(S);
                        Paint::DrawConvex(C, 0, 255, 0, 255);
                        return true;
                    }
                    Wait(NormalRandom(200, 10.0f));
                }
            }
        }
    }
    return false;
}

bool DropFish()
{
    if (!Inventory::IsOpen())
        Inventory::Open();

    Fished.Increment(Inventory::Count(Items));

    bool ShiftClick = (Settings::GetSettingBit(SHIFT_CLICK_DROP_SETTING, SHIFT_CLICK_DROP_BIT_POS));
    if ((ShiftClick) && !IsKeyDown(KEY_SHIFT))
        Interact::DownKey(KEY_SHIFT);

    std::vector<std::int32_t> Fish;
    for (const auto& I : Items)
    {
        auto Temp = Inventory::GetIndicesOf(I);
        Fish.insert(Fish.end(), Temp.begin(), Temp.end());
    }
    std::sort(Fish.begin(), Fish.end());

    for (auto& F : Fish)
    {
        if (Inventory::IsItemSelected())
            Inventory::InteractItemByIndex(F);
        Inventory::DropItemByIndex(F);
    }

    if ((ShiftClick) && IsKeyDown(KEY_SHIFT))
        Interact::UpKey(KEY_SHIFT);

    return (!Inventory::ContainsAny(Items));
}

bool Loop()
{
    BreakHandler::Break(false);
    if (LoopCD.IsFinished())
    {
        RandomHandler::Check();
        LoopCD.SetTime(NormalRandom(10000, 0.40f));
        if (Login::IsLoggedIn())
        {
            if (PrintCD.IsFinished())
                PrintStatus();

            if (!Inventory::ContainsAny(Tools))
            {
                Debug::Fatal << "Tools aren't found in the Players Inventory, you need ";
                for (auto& T : Tools)
                    Debug::Fatal << T << " ";
                Debug::Fatal << "in the Players Inventory, stopping";
                return false;
            }

            if ((Inventory::IsFull()) && (Inventory::ContainsAny(Items)))
            {
                Debug::Info << "Inventory Full, attempting to empty Inventory" << std::endl;
                GainFocus();
                Debug::Info << "Gained focus" << std::endl;
                DropFish();
                Debug::Info << "DropFish ended, about to Wait" << std::endl;
                Wait(NormalRandom(2000, 0.05f));
            }

            Player P = Players::GetLocal();
            if ((P.GetAnimationID() == -1) && (!Minimap::GetDestination().IsNegative()))
            {
                Debug::Info << "Not Fishing, gaining focus and attempting to Fish" << std::endl;
                GainFocus();
                if (FishSpot(Action))
                {
                    Debug::Info << "Successfully Fished";
                    if (UniformRandom() < TabOutChance)
                    {
                        Debug::Info << ", alt-tabbing" << std::endl;
                        LoseFocus();
                    } else
                        Debug::Info << std::endl;
                    Wait(NormalRandom(2000, 0.05f));
                }
            }

            return true;

        } else
        {
            Debug::Info << "Logging in..." << std::endl;
            if (Login::LoginPlayer() <= 0)
            {
                Debug::Fatal << Login::LoginPlayer() << std::endl;
                Debug::Fatal << "LoginPlayer Failed, stopping." << std::endl;
                return false;
            } else
                return true;
        }
    }
    return true;
}
