# DialogAndQuest Plugin — Integration Guide

This document walks through every step required to integrate the DialogAndQuest plugin into an Unreal Engine 5 C++ project. It covers module setup, interface implementation, data authoring, UI wiring, persistence, and voiceover.

---

## Table of Contents

1. [Prerequisites](#1-prerequisites)
2. [Module Setup](#2-module-setup)
3. [GameMode Integration](#3-gamemode-integration)
4. [NPC / Dialog Actor Setup](#4-npc--dialog-actor-setup)
5. [Player Controller Setup](#5-player-controller-setup)
6. [Quest Giver Actors](#6-quest-giver-actors)
7. [Authoring Dialog Data](#7-authoring-dialog-data)
8. [Authoring Quest Data](#8-authoring-quest-data)
9. [Quest State Machine](#9-quest-state-machine)
10. [Dialog Conditions & Consequences](#10-dialog-conditions--consequences)
11. [UI Setup](#11-ui-setup)
12. [Persistence (Save/Load)](#12-persistence-saveload)
13. [Voiceover Integration](#13-voiceover-integration)
14. [Optional: Skill Checks & Item Requirements](#14-optional-skill-checks--item-requirements)
15. [Area-Based Quest Validators](#15-area-based-quest-validators)
16. [Class Reference Summary](#16-class-reference-summary)
17. [Migration from v1 (DataTable-only)](#17-migration-from-v1-datatable-only)

---

## 1. Prerequisites

- Unreal Engine 5.x C++ project
- `GameplayTags` plugin enabled (shipped with UE)
- Basic understanding of UE replication (dedicated server model)

## 2. Module Setup

### 2.1 Add the plugin

Copy or add as a git submodule into your project's `Plugins/` directory:

```
YourProject/
  Plugins/
    DialogAndQuest/
      DialogAndQuestPlugin.uplugin
      Source/
        DialogAndQuestPlugin/     (Runtime)
        DialogAndQuestPluginEditor/ (Editor-only)
```

### 2.2 Add module dependency

In your game module's `.Build.cs`:

```csharp
PublicDependencyModuleNames.AddRange(new string[]
{
    "DialogAndQuestPlugin",
    "GameplayTags",   // if using skill checks or botch tags
});
```

### 2.3 Verify

Regenerate project files. You should now be able to `#include` any plugin header.

---

## 3. GameMode Integration

Your GameMode is the **server-side authority** for all dialog and quest master data. It owns the global registries that NPCs and players query.

### 3.1 Implement `IDialogGameModeInterface`

```cpp
// MyGameMode.h
#pragma once

#include "GameFramework/GameModeBase.h"
#include "Interfaces/DialogGameModeInterface.h"
#include "MyGameMode.generated.h"

UCLASS()
class AMyGameMode : public AGameModeBase, public IDialogGameModeInterface
{
    GENERATED_BODY()

protected:
    UPROPERTY()
    TObjectPtr<UDialogMainComponent> DialogComponent;

    UPROPERTY()
    TObjectPtr<UQuestMainComponent> QuestComponent;

public:
    AMyGameMode();

    // IDialogGameModeInterface
    virtual UDialogMainComponent* GetMainDialogComponent() override { return DialogComponent; }
    virtual UQuestMainComponent* GetMainQuestComponent() override { return QuestComponent; }
};
```

### 3.2 Create components in constructor

```cpp
// MyGameMode.cpp
AMyGameMode::AMyGameMode()
{
    DialogComponent = CreateDefaultSubobject<UDialogMainComponent>("DialogMainComponent");
    QuestComponent = CreateDefaultSubobject<UQuestMainComponent>("QuestMainComponent");
}
```

### 3.3 Load dialog and quest data at startup

In `BeginPlay()` or an initialization function, register all dialog and quest data. You have two options:

**Option A — DataTable (legacy)**

```cpp
void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Load dialog DataTables
    DialogComponent->AddTopicFromDataTable(MyDialogTopicDT);
    DialogComponent->AddBundleFromDataTable(MyDialogBundleDT);
    DialogComponent->AddMetaBundleFromDataTable(MyDialogMetaBundleDT);

    // Load quest DataTables
    QuestComponent->AddQuestFromDataTable(MyQuestDT);
}
```

**Option B — UDialogAsset / UQuestAsset (recommended)**

```cpp
void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Load from assets
    for (UDialogAsset* Asset : DialogAssets)
        DialogComponent->AddFromAsset(Asset);

    for (UQuestAsset* Asset : QuestAssets)
        QuestComponent->AddQuestFromAsset(Asset);
}
```

You can configure the asset arrays as `UPROPERTY(EditAnywhere)` on a Blueprint subclass of your GameMode, or load them programmatically from a database.

---

## 4. NPC / Dialog Actor Setup

Any actor that a player can talk to must implement `IDialogInterface` and `IQuestGiverInterface` (if it gives/advances quests).

### 4.1 Implement `IDialogInterface`

```cpp
// MyNPC.h
#pragma once

#include "GameFramework/Character.h"
#include "Interfaces/DialogInterface.h"
#include "Interfaces/QuestGiverInterface.h"
#include "MyNPC.generated.h"

UCLASS()
class AMyNPC : public ACharacter, public IDialogInterface, public IQuestGiverInterface
{
    GENERATED_BODY()

protected:
    UPROPERTY()
    TObjectPtr<UDialogComponent> Dialog;

    UPROPERTY()
    TObjectPtr<UQuestGiverComponent> QuestComponent;

    UPROPERTY(EditAnywhere, Category = "Dialog")
    int64 DialogMetaBundleID = 0;

public:
    AMyNPC();

    // IDialogInterface — required overrides
    virtual float GetRelation(AActor* RelationWithActor) const override;
    virtual UDialogComponent* GetDialogComponent() const override { return Dialog; }
    virtual FString GetRelationString(float Relation) const override;
    virtual bool HasDialog() const override { return DialogMetaBundleID != 0; }
    virtual FText GetCharacterNameForDialog() const override;

    // IQuestGiverInterface
    virtual UQuestGiverComponent* GetQuestGiverComponent() const override { return QuestComponent; }

    void InitDialog();
    void InitQuest();
};
```

### 4.2 Constructor: create components

```cpp
AMyNPC::AMyNPC()
{
    Dialog = CreateDefaultSubobject<UDialogComponent>("DialogComponent");
    Dialog->SetIsReplicated(true);   // Dialog data replicates to clients

    QuestComponent = CreateDefaultSubobject<UQuestGiverComponent>("QuestComponent");
    QuestComponent->SetIsReplicated(false);  // Quest validation is server-only
}
```

### 4.3 Initialize dialog on the server

```cpp
void AMyNPC::InitDialog()
{
    if (!HasAuthority() || !HasDialog())
        return;

    Dialog->InitDialogFromID(DialogMetaBundleID);

    // Retry if data isn't loaded yet (e.g., GameMode still loading DataTables)
    if (!Dialog->IsValid())
    {
        FTimerHandle Handle;
        GetWorld()->GetTimerManager().SetTimer(Handle, this, &AMyNPC::InitDialog, 1.0f, false);
    }
}
```

### 4.4 Register quest giver steps

If this NPC can advance specific quests, register which quest steps it can validate:

```cpp
void AMyNPC::InitQuest()
{
    if (!HasAuthority() || !QuestComponent)
        return;

    // This NPC can validate step 0 and step 1 of quest 1001
    QuestComponent->AddValidatableSteps(1001, {0, 1});
}
```

### 4.5 Implement `GetRelation`

The relation value (0.0–1.0) gates which dialog topics appear. The interpretation is game-specific:

```cpp
float AMyNPC::GetRelation(AActor* RelationWithActor) const
{
    // Your faction/reputation system here
    return 0.5f; // Indifferent by default
}

FString AMyNPC::GetRelationString(float Relation) const
{
    if (Relation < 0.25f) return "Threatening";
    if (Relation < 0.50f) return "Indifferent";
    if (Relation < 0.75f) return "Amiable";
    return "Ally";
}
```

---

## 5. Player Controller Setup

The PlayerController is the **quest bearer** and the **dialog display driver**. It needs up to four interfaces depending on which features you use.

### 5.1 Required interfaces

| Interface | Purpose | Required? |
|-----------|---------|-----------|
| `IQuestBearerInterface` | Tracks quest progress | Yes |
| `IDialogDisplayInterface` | Drives dialog UI | Yes |
| `IDialogConsequenceInterface` | Handles dialog consequences (faction, state changes) | If using consequences |
| `IDialogSkillCheckInterface` | Evaluates skill/item checks for topic conditions | If using skill-gated dialog |

### 5.2 Minimal setup

```cpp
// MyPlayerController.h
#pragma once

#include "GameFramework/PlayerController.h"
#include "Interfaces/QuestBearerInterface.h"
#include "Interfaces/DialogDisplayInterface.h"
#include "Interfaces/DialogConsequenceInterface.h"
#include "Interfaces/DialogSkillCheckInterface.h"
#include "MyPlayerController.generated.h"

UCLASS()
class AMyPlayerController : public APlayerController,
    public IQuestBearerInterface,
    public IDialogDisplayInterface,
    public IDialogConsequenceInterface,
    public IDialogSkillCheckInterface
{
    GENERATED_BODY()

protected:
    UPROPERTY()
    TObjectPtr<UQuestBearerComponent> QuestBearerComponent;

public:
    AMyPlayerController();

    // IQuestBearerInterface
    virtual UQuestBearerComponent* GetQuestBearerComponent() override { return QuestBearerComponent; }
    virtual const UQuestBearerComponent* GetQuestBearerComponentConst() const override { return QuestBearerComponent; }

    // IDialogDisplayInterface — implement in Blueprint or C++
    virtual void ForceDisplayTextInDialog_Implementation(const FString& TextString) override;

    // IDialogConsequenceInterface
    virtual void HandleDialogConsequence(const FDialogConsequence& Consequence, AActor* DialogActor) override;

    // IDialogSkillCheckInterface
    virtual float EvaluateSkillCheck(const FGameplayTag& SkillTag, const AActor* CheckingActor) const override;
    virtual bool HasRequiredItems(const TArray<int32>& RequiredItems, const AActor* CheckingActor) const override;
};
```

### 5.3 Constructor

```cpp
AMyPlayerController::AMyPlayerController()
{
    QuestBearerComponent = CreateDefaultSubobject<UQuestBearerComponent>("QuestBearerComponent");
    QuestBearerComponent->SetIsReplicated(true);
}
```

### 5.4 Bind quest save on update

Connect the `QuestUpdateDispatcher` to your save logic:

```cpp
void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    QuestBearerComponent->QuestUpdateDispatcher.AddDynamic(this, &AMyPlayerController::OnQuestUpdated);
}

void AMyPlayerController::OnQuestUpdated(int64 QuestID, int32 StepID, EQuestState NewState)
{
    // Persist to your backend
    SaveQuestToBackend(QuestID, StepID, static_cast<uint8>(NewState));
}
```

### 5.5 Implement consequence handler

```cpp
void AMyPlayerController::HandleDialogConsequence(const FDialogConsequence& Consequence, AActor* DialogActor)
{
    if (Consequence.MentionQuestID != 0)
        GetQuestBearerComponent()->MentionQuest(Consequence.MentionQuestID);

    if (Consequence.QuestID != 0 && Consequence.NewQuestState != EQuestState::Unknown)
    {
        // Delegate to appropriate state transition
        switch (Consequence.NewQuestState)
        {
        case EQuestState::Mentioned: GetQuestBearerComponent()->MentionQuest(Consequence.QuestID); break;
        case EQuestState::Accepted:  GetQuestBearerComponent()->AuthorityAddQuest(Consequence.QuestID); break;
        case EQuestState::Achieved:  GetQuestBearerComponent()->AchieveQuest(Consequence.QuestID); break;
        case EQuestState::Completed: GetQuestBearerComponent()->CompleteQuest(Consequence.QuestID); break;
        case EQuestState::Botched:   GetQuestBearerComponent()->BotchQuest(Consequence.QuestID); break;
        default: break;
        }
    }

    if (Consequence.FactionDelta != 0.f)
        ApplyFactionDelta(DialogActor, Consequence.FactionDelta);
}
```

---

## 6. Quest Giver Actors

Any actor that advances quests (NPCs, objects, triggers) needs `IQuestGiverInterface` and a `UQuestGiverComponent`.

The component stores which `{QuestID, StepIDs}` this actor can validate. When a player interacts, the system checks the actor's component to see if it can validate the next step of the player's quest.

```cpp
// On your NPC or interactable actor:
QuestComponent->AddValidatableSteps(/*QuestID=*/ 1001, /*Steps=*/ {0, 1, 2});
```

Step 0 is special — it means "this actor can give the quest to a player who doesn't know it yet."

---

## 7. Authoring Dialog Data

### 7.1 Using UDialogAsset (recommended)

1. In the Content Browser, right-click → **Gameplay** → **Dialog Asset**
2. Fill in:
   - **AssetName**: identifier (e.g., "Baker_Village")
   - **GoodGreeting** / **BadGreeting**: the NPC's opening line based on relation
   - **MinimumRelation**: threshold (0.0–1.0)
   - **Topics**: add entries with unique `Id`, `Topic` keyword, `TopicText`, and conditions
3. On your GameMode, reference the asset and call `DialogComponent->AddFromAsset(Asset)` at startup

### 7.2 Using DataTables (legacy)

Create three DataTables:

| DataTable | Row struct | Purpose |
|-----------|-----------|---------|
| Topics | `FDialogTopicStruct` | Individual dialog entries |
| Bundles | `FDialogTopicBundleStruct` | Groups of topic IDs |
| MetaBundles | `FDialogTopicMetaBundleStruct` | Groups of bundles + greetings |

Load them in order:

```cpp
DialogComponent->AddTopicFromDataTable(TopicsDT);
DialogComponent->AddBundleFromDataTable(BundlesDT);
DialogComponent->AddMetaBundleFromDataTable(MetaBundlesDT);
```

Assign the MetaBundle ID to each NPC's `DialogMetaBundleID`.

### 7.3 Topic structure

Each `FDialogTopicStruct` entry:

| Field | Type | Description |
|-------|------|-------------|
| `Id` | int64 | Unique topic ID |
| `Topic` | FString | The keyword players click on |
| `TopicText` | FText | NPC's response (supports `<DialogLink>` hyperlinks) |
| `TopicCondition` | FDialogTopicCondition | When this topic is visible |
| `Consequence` | FDialogConsequence | What happens when clicked |
| `QuestRelation` | FQuestValidatableSteps | Legacy quest step progression |
| `VoiceoverCue` | TSoftObjectPtr\<USoundBase\> | Audio to play |

### 7.4 Hyperlink keywords

In `TopicText`, any word that matches another topic's `Topic` keyword will automatically become a clickable hyperlink. For example, if topics "bread" and "flour" exist:

> "I am the local baker. My bread is the best in town, though the price of flour has been terrible lately."

"bread" and "flour" will render as clickable links that display their respective topic text.

---

## 8. Authoring Quest Data

### 8.1 Using UQuestAsset (recommended)

1. Content Browser → **Gameplay** → **Quest Asset**
2. Fill in:
   - **QuestID**: unique int64
   - **QuestTitle**: journal display name
   - **QuestDescription**: full text shown when accepted
   - **MentionedDescription**: rumor text shown before formal acceptance
   - **Steps**: ordered array of `FQuestStep`

### 8.2 Quest step structure

| Field | Type | Description |
|-------|------|-------------|
| `QuestID` | int64 | Parent quest reference |
| `QuestSubID` | int32 | Unique step ID within the quest |
| `StepTitle` | FText | Short step name |
| `StepDescription` | FText | What the player should do |
| `FinishingStep` | bool | True = completing this step finishes the quest |
| `RewardClass` | TSubclassOf\<UQuestRewardData\> | Reward blueprint |
| `NecessaryItems` | TArray\<int32\> | Items that must be turned in |
| `NecessaryCoins` | float | Money that must be turned in |
| `ItemTurnInDialog` | FText | Text displayed on turn-in |
| `StepType` | EQuestStepType | Linear / Branch / Parallel / Optional |
| `NextStepIDs` | TArray\<int32\> | For non-linear steps |
| `VoiceoverCue` | TSoftObjectPtr\<USoundBase\> | Step turn-in voiceover |

### 8.3 Rewards

Subclass `UQuestRewardData` or use it directly:

```cpp
UCLASS(Blueprintable)
class UMyQuestReward : public UQuestRewardData
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere) TSubclassOf<AActor> SpawnedRewardActor;
};
```

Override `IQuestBearerInterface::GrantReward()` on your PlayerController to process rewards.

---

## 9. Quest State Machine

Quests follow a six-state lifecycle:

```
Unknown ──► Mentioned ──► Accepted ──► Achieved ──► Completed
                │              │            │
                │              │            │
                └──────────────┴────────────┴──► Botched
```

| State | Meaning | Journal display |
|-------|---------|-----------------|
| `Unknown` | Player has never heard of this quest | Not shown |
| `Mentioned` | Player has heard a rumor (overheard, read a note, NPC mentioned it) | Shows `MentionedDescription` only |
| `Accepted` | Player formally took the quest | Shows full description + steps |
| `Achieved` | All objectives done, awaiting turn-in | Shows steps as completed |
| `Completed` | Quest fully done, rewards granted | Marked as finished |
| `Botched` | Irrecoverably failed | Shown with strikethrough |

### State transitions (server-only)

```cpp
// From game code or dialog consequences:
QuestBearer->MentionQuest(QuestID);         // Unknown → Mentioned
QuestBearer->AuthorityAddQuest(QuestID);    // Unknown/Mentioned → Accepted
QuestBearer->AchieveQuest(QuestID);         // Accepted → Achieved
QuestBearer->CompleteQuest(QuestID);        // Achieved/Accepted → Completed
QuestBearer->BotchQuest(QuestID);           // Any non-terminal → Botched
```

### Botch conditions

Define `FQuestBotchCondition` entries on the quest with a `FGameplayTag BotchTag`. Your game monitors world events (NPC death, item destroyed) and calls `BotchQuest()` when a matching condition occurs.

---

## 10. Dialog Conditions & Consequences

### Topic conditions (`FDialogTopicCondition`)

Control when a topic is visible to the player:

| Field | Effect |
|-------|--------|
| `MinimumRelation` | Minimum faction relation value |
| `QuestId` + `MinimumStepID` + `StepCondition` | Legacy step-based quest condition |
| `RequiredQuestState` | Topic shows only when quest is in this state |
| `RequiredItems` | Player must carry these item IDs |
| `SkillCheckTag` + `MinimumSkillValue` | Skill check via `IDialogSkillCheckInterface` |
| `bConsumeOnUse` | Topic disappears after first click |

### Topic consequences (`FDialogConsequence`)

Triggered when a player clicks the topic:

| Field | Effect |
|-------|--------|
| `QuestID` + `NewQuestState` | Transition a quest to a new state |
| `FactionDelta` | Adjust NPC relation |
| `MentionQuestID` | "Mention" another quest (Unknown → Mentioned) |

Example: an NPC who gossips about a missing merchant. Clicking "merchant" mentions quest 2001:
- `MentionQuestID = 2001`

Example: an NPC who formally offers a quest. Clicking "accept" moves quest 1001 to Accepted:
- `QuestID = 1001`, `NewQuestState = Accepted`

---

## 11. UI Setup

The plugin provides C++ widget base classes. You create Blueprint subclasses for visual layout.

### Required widgets

| C++ class | Purpose | Blueprint binding |
|-----------|---------|-------------------|
| `UDialogWindow` | Main dialog container | `BindWidget` for Footer, Header, TopicList, TopicText, WidgetSwitcher |
| `UDialogHeaderWidget` | NPC name, relation, action buttons | `BindWidgetOptional` for Trade/Give/Train/Bank/Repair buttons |
| `UDialogFooterWidget` | Exit button | — |
| `UDialogTopicWidget` | Clickable topic list sidebar | `BlueprintImplementableEvent`: AddTopicData, ClearList |
| `UDialogTextWidget` | Main text display area | `BlueprintImplementableEvent`: AddTopicData, ClearList |
| `UDialogTextChunkWidget` | Individual text entry in ListView | `BlueprintImplementableEvent`: InitData |
| `UQuestJournalWindow` | Journal container | Set up ListWidget + DetailsWidget |
| `UQuestJournalListWidget` | Quest list sidebar | `BlueprintImplementableEvent`: AddQuestTitleData, ClearList |
| `UQuestJournalDetailsWidget` | Quest detail view | `BlueprintImplementableEvent`: AddQuestData, ClearData |

### Setup steps

1. Create a **Blueprint Widget** inheriting `UDialogWindow`
2. Add child widgets matching the `BindWidget` names: `Footer`, `Header`, `TopicList`, `TopicText`, `WidgetSwitcher`
3. Create Blueprint Widgets for each sub-widget inheriting the correct C++ class
4. Implement the `BlueprintImplementableEvent` functions for visual layout
5. On your PlayerController, implement `IDialogDisplayInterface::CreateDialogWindow` to instantiate and show the widget
6. Call `DialogWindow->InitDialogWindow(DialogComponent, NPCActor)` to start a conversation

### Hyperlink decorator

Add `URichInlineHyperlinkDecorator` to your `URichTextBlock`'s decorator list. Override `ClickFun` in Blueprint to call `DialogWindow->DisplayDialogTopicFromString(ID)`.

---

## 12. Persistence (Save/Load)

### Save contract

Listen to `QuestUpdateDispatcher` (broadcasts `QuestID`, `StepID`, `EQuestState`). Persist these three values per quest per player.

```json
{ "q": 1001, "s": 2, "st": 3 }
```

Where `st` is the `EQuestState` as uint8 (0=Unknown, 1=Mentioned, 2=Accepted, 3=Achieved, 4=Completed, 5=Botched).

### Load contract

On player login, call:

```cpp
QuestBearerComponent->AuthoritySetupQuestData(
    QuestID,
    StepID,
    static_cast<EQuestState>(SavedState)
);
```

For backward compatibility with v1 saves that lack a state field, the default `InitialState` parameter is `EQuestState::Accepted`.

---

## 13. Voiceover Integration

Every `FDialogTopicStruct`, `FQuestStep`, and `FDialogTopicMetaBundleStruct` (greetings) has voiceover fields:

| Field | Type | Use |
|-------|------|-----|
| `VoiceoverCue` | `TSoftObjectPtr<USoundBase>` | UE sound asset (loaded async) |
| `VoiceoverEventName` | `FName` | External middleware event (Wwise/FMOD) |
| `VoiceoverDuration` | `float` | Subtitle hint in seconds (0 = use asset duration) |

### Delegates on `UDialogComponent`

| Delegate | Signature | When fired |
|----------|-----------|------------|
| `OnVoiceoverRequested` | `(USoundBase* Sound, float Duration)` | Topic or greeting has a VO cue and the asset has finished async loading |
| `OnVoiceoverStop` | `()` | Before every new topic starts, and in `CloseWindow()` |
| `OnMiddlewareVoiceoverRequested` | `(FName EventName)` | Topic has `VoiceoverEventName` set (fires synchronously on click) |

Greeting VOs (`GoodGreetingVoiceover` / `BadGreetingVoiceover`) are replicated to clients as string paths and fire `OnVoiceoverRequested` automatically when the dialog window opens. **No extra wiring is needed for greetings.**

### Naming convention (required for auto-link scripts)

```
Content/Sounds/Dialog/{NPCName}/
  VO_{NPCName}_Greeting_Good.uasset
  VO_{NPCName}_Greeting_Bad.uasset
  VO_{NPCName}_Topic_{TopicKeyword}.uasset
```

### Step 1 — Add a VoiceAudioComponent to the NPC Blueprint

1. Open your NPC Blueprint (e.g. `BP_Baker`).
2. Add an **Audio Component**, name it `VoiceAudioComponent`.
3. Attach it to the `head` socket on the skeletal mesh.
4. Set **Auto Activate → false**.
5. Set an appropriate attenuation preset (short range, no reverb).

### Step 2 — Bind to `OnVoiceoverRequested` on BeginPlay

In the NPC Blueprint's Event Graph:

```
Event BeginPlay
  → Get Dialog Component
  → Bind Event to OnVoiceoverRequested
        (SoundCue USoundBase*, Duration float)
        → Stop VoiceAudioComponent          ← interrupt previous line
        → Set Sound (VoiceAudioComponent, SoundCue)
        → Play (VoiceAudioComponent)
        → Set local variable "LastVODuration" = Duration
```

### Step 3 — Bind to `OnVoiceoverStop`

```
Event BeginPlay (continued)
  → Bind Event to OnVoiceoverStop
        → Stop VoiceAudioComponent
        → Hide subtitle widget (if any)
```

This fires automatically when:
- The player clicks a new topic (before the new VO starts)
- `CloseWindow()` is called

### Step 4 — Create `WBP_Subtitle` (optional subtitle overlay)

1. Create a new Widget Blueprint: `WBP_Subtitle`.
2. Add a **Vertical Box** with a **Text Block** (`SubtitleText`) and an optional **Text Block** (`SpeakerName`).
3. Expose two Blueprint functions:
   - `ShowSubtitle(SpeakerName FText, SubtitleText FText, Duration float)` — sets text, makes widget visible, starts a timer that calls `HideSubtitle()` after `Duration` seconds (use `5.0f` fallback when Duration == 0).
   - `HideSubtitle()` — hides the widget.
4. Add the widget to your HUD layout at the bottom of the screen.

### Step 5 — Drive subtitles from `UI_DialogWindow`

In `UI_DialogWindow` Event Graph:

```
Event Construct
  → Get Dialog Component (from owning NPC)
  → Bind Event to OnVoiceoverRequested
        (SoundCue, Duration)
        → Get Owning HUD → Cast to BP_MainHUD
        → Get Subtitle Widget
        → ShowSubtitle(
              SpeakerName = DialogComponent→GetDialogName(),
              SubtitleText = TopicText (cache from last AddTopicText call),
              Duration = Duration)

  → Bind Event to OnVoiceoverStop
        → Get Subtitle Widget → HideSubtitle()
```

`FDialogTextData` now carries `VoiceoverCue` and `VoiceoverDuration` — in `UI_DialogTextChunk`'s `InitData` event you can read `ItemData.VoiceoverDuration` for fine-grained per-chunk subtitle timing.

### Step 6 — External middleware (Wwise / FMOD)

Bind to `OnMiddlewareVoiceoverRequested` instead of `OnVoiceoverRequested` when using an external audio system:

```
Bind Event to OnMiddlewareVoiceoverRequested
  (EventName FName)
  → AkComponent → Post Ak Event (EventName)
  // OR
  → FMOD → Play Event By Name (EventName)
```

Both `OnVoiceoverRequested` and `OnMiddlewareVoiceoverRequested` fire independently — a topic can have both a `VoiceoverCue` (for editor preview) and a `VoiceoverEventName` (for production audio).

### Step 7 — Assign assets to topics

Open each `DA_NPC_*` asset (UDialogAsset):

- **Greeting|Voiceover** category → assign `GoodGreetingVoiceover` and `BadGreetingVoiceover`.
- **Topics** array → expand each entry → **Dialog|Voiceover** → assign `VoiceoverCue`. Leave `VoiceoverDuration = 0` unless you have a precise clip length for subtitles.

### Step 8 — Verify in Multiplayer PIE

1. **Play as Listen Server + 1 Client**.
2. From the client window, interact with an NPC.
3. Confirm: greeting VO plays from the NPC head socket, subtitle appears, clicking a new topic stops the previous VO and starts the new one, closing dialog stops VO.
4. Check **Output Log** for any `LogAssetManager` warnings about missing soft object paths.

---

## 14. Optional: Skill Checks & Item Requirements

Implement `IDialogSkillCheckInterface` on your PlayerController:

```cpp
float AMyPlayerController::EvaluateSkillCheck(const FGameplayTag& SkillTag, const AActor* CheckingActor) const
{
    // Map tag to your skill system
    if (SkillTag.MatchesTag(FGameplayTag::RequestGameplayTag("Skill.Persuasion")))
        return GetPersuasionSkill();
    return 0.f;
}

bool AMyPlayerController::HasRequiredItems(const TArray<int32>& RequiredItems, const AActor* CheckingActor) const
{
    for (int32 ItemID : RequiredItems)
    {
        if (!InventoryContains(ItemID))
            return false;
    }
    return true;
}
```

Then on dialog topics, set `TopicCondition.SkillCheckTag` to e.g. `"Skill.Persuasion"` with `MinimumSkillValue = 50.0`.

---

## 15. Area-Based Quest Validators

`AAreaQuestValidator` is a box trigger that progresses a quest when a player enters it.

1. Place in your level
2. Set `ValidatableSteps.QuestID` and `ValidatableSteps.Steps` in the details panel
3. The actor automatically calls `TryProgressQuest` on overlapping players

Use for "go to location" quest steps.

---

## 16. Class Reference Summary

### Components

| Component | Owner | Replicates | Purpose |
|-----------|-------|------------|---------|
| `UDialogMainComponent` | GameMode | No | Global dialog registry |
| `UQuestMainComponent` | GameMode | No | Global quest registry |
| `UDialogComponent` | NPC Actor | Yes | Per-NPC dialog data |
| `UQuestBearerComponent` | PlayerController | Yes (owner-only) | Per-player quest progress |
| `UQuestGiverComponent` | NPC/Trigger | No | Quest step validation |

### Interfaces

| Interface | Implement on | Pure virtuals |
|-----------|-------------|---------------|
| `IDialogGameModeInterface` | GameMode | `GetMainDialogComponent`, `GetMainQuestComponent` |
| `IDialogInterface` | NPC Actor | `GetRelation`, `GetDialogComponent`, `GetRelationString`, `HasDialog`, `GetCharacterNameForDialog` |
| `IQuestGiverInterface` | NPC/Trigger | `GetQuestGiverComponent` |
| `IQuestBearerInterface` | PlayerController | `GetQuestBearerComponent`, `GetQuestBearerComponentConst` |
| `IDialogDisplayInterface` | PlayerController | `ForceDisplayTextInDialog` |
| `IDialogConsequenceInterface` | PlayerController | `HandleDialogConsequence` |
| `IDialogSkillCheckInterface` | PlayerController | `EvaluateSkillCheck` |

### Data Assets

| Asset | Purpose |
|-------|---------|
| `UDialogAsset` | Unified dialog data (replaces 3 DataTables) |
| `UQuestAsset` | Unified quest data (replaces quest DataTable) |

### Key Structs

| Struct | Purpose |
|--------|---------|
| `FDialogTopicStruct` | Single dialog topic |
| `FDialogTopicCondition` | Visibility conditions for a topic |
| `FDialogConsequence` | World-state changes on topic click |
| `FQuestMetaData` | Complete quest definition |
| `FQuestStep` | Single quest objective |
| `FQuestProgressData` | Player's progress on a quest |
| `FQuestBotchCondition` | Condition that fails a quest |
| `UQuestRewardData` | Reward granted on step completion |

### Enums

| Enum | Values |
|------|--------|
| `EQuestState` | Unknown, Mentioned, Accepted, Achieved, Completed, Botched |
| `EQuestStepType` | Linear, Branch, Parallel, Optional |
| `EQuestStepConditionType` | Equal, Lesser, LesserEqual, Greater, GreaterEqual |

---

## 17. Migration from v1 (DataTable-only)

If upgrading from the pre-2.0 DataTable-only version:

1. **FString → FText**: `StepTitle`, `StepDescription`, `ItemTurnInDialog`, `QuestTitle`, `TopicText`, `GoodGreetingDialog`, `BadGreetingDialog` are now `FText`. Update your DataTable row definitions accordingly. Existing DataTable CSV exports need the values wrapped in `NSLOCTEXT()` or `INVTEXT()` macros, or simply re-imported.

2. **Quest state**: `FQuestProgressData::Finished` is replaced by `EQuestState State`. Use `IsFinished()` helper for backward-compatible checks. Update your save/load to persist the state uint8 alongside quest ID and step.

3. **GetDialogTopic → GetDialogTopicSafe**: The old `GetDialogTopic(int64)` returned a reference and crashed on miss. The new `GetDialogTopicSafe(int64)` returns a pointer (null on miss). Update call sites to check for null.

4. **QuestUpdateDispatcher signature**: Changed from `(int64 QuestID, int32 StepID)` to `(int64 QuestID, int32 StepID, EQuestState NewState)`. Update all bound delegates.

5. **IsValid() semantics**: No longer compares greeting strings against `"Error"`. Uses an internal `bDialogInitialized` flag. Behavior is the same from the caller's perspective — no code changes needed.

6. **New optional features**: Consequences, skill checks, voiceover, and assets are all additive. Existing DataTable-based setups continue to work without modification.

