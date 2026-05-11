#pragma once

#include "CoreMinimal.h"
#include "Components/DialogComponent.h"
#include "UObject/Interface.h"
#include "DialogInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UDialogInterface : public UInterface
{
	GENERATED_BODY()
};

class DIALOGANDQUESTPLUGIN_API IDialogInterface
{
	GENERATED_BODY()

public:
	/**
	 * Returns the normalised relation of this actor toward RelationWithActor.
	 * Range [0, 1], clamped against the visible faction window [-1000, +1000]:
	 *   0.0  = Scowling  (raw faction <= -1000)
	 *   0.5  = Indifferent (raw faction == 0)
	 *   1.0  = Ally      (raw faction >= +1000)
	 * Implementors must clamp output to [0, 1]; consumers must not assume values outside that range.
	 */
	UFUNCTION(BlueprintCallable)
	virtual float GetRelation(AActor* RelationWithActor) const = 0;

	UFUNCTION(BlueprintCallable)
	virtual UDialogComponent* GetDialogComponent() const = 0;

	UFUNCTION(BlueprintCallable)
	virtual FString GetRelationString(float Relation) const = 0;

	virtual bool HasDialog() const = 0;

	/**
	 * Returns the bad greeting text to display when the NPC is hostile to RequestingActor.
	 * Default returns the static BadGreeting stored in the DialogComponent.
	 * Game code can override this to return a dynamically-selected line (e.g. faction-aware
	 * insults based on race, class, or personal reputation) without touching the plugin.
	 *
	 * @param Relation          Normalised relation value [0,1] at the time of dialog opening.
	 * @param RequestingActor   The player pawn opening the dialog window (may be nullptr).
	 */
	virtual FText GetContextualBadGreeting(float Relation, AActor* RequestingActor) const;


	virtual bool CanTrade() const;

	virtual bool CanGive() const;

	virtual bool CanTrain() const;

	virtual bool CanBank() const;

	virtual bool CanRepair() const;

	virtual FText GetCharacterNameForDialog() const = 0;

	virtual float GetMaxInteractionDistance() const;
};
