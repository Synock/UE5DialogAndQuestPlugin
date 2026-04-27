
#include "Actors/AreaQuestValidator.h"

#include "Interfaces/QuestBearerInterface.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AAreaQuestValidator::AAreaQuestValidator(): AActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	if (HasAuthority())
	{
		QuestComponent = CreateDefaultSubobject<UQuestGiverComponent>("QuestComponent");
		QuestComponent->SetIsReplicated(false); // Enable replication by default

		BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
		BoxComponent->SetBoxExtent(FVector(200.f, 200.f, 200.f));
		BoxComponent->bDynamicObstacle = false;
		BoxComponent->SetupAttachment(GetRootComponent());
		BoxComponent->SetGenerateOverlapEvents(true);
		BoxComponent->SetIsReplicated(false);
		BoxComponent->SetCanEverAffectNavigation(false);
		SetRootComponent(BoxComponent);

		OnActorBeginOverlap.AddDynamic(this, &AAreaQuestValidator::OnOverlapBegin);
		OnActorEndOverlap.AddDynamic(this, &AAreaQuestValidator::OnOverlapEnd);
	}
}

// Called when the game starts or when spawned
void AAreaQuestValidator::BeginPlay()
{
	Super::BeginPlay();

	if (QuestComponent && QuestAsset && QuestAsset->QuestID != 0)
	{
		QuestComponent->AddValidatableSteps(QuestAsset->QuestID, {StepID});
	}
}

void AAreaQuestValidator::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!HasAuthority())
		return;

	TryProgressQuest(OtherActor);
}

void AAreaQuestValidator::OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor)
{
}

bool AAreaQuestValidator::TryProgressQuest(AActor* OtherActor)
{
	if (!HasAuthority())
		return false;

	if (!QuestAsset || QuestAsset->QuestID == 0)
		return false;

	const APawn* ActorAsPawn = Cast<APawn>(OtherActor);
	if (!ActorAsPawn)
		return false;

	APlayerController* PlayerController = Cast<APlayerController>(ActorAsPawn->GetController());
	if (!PlayerController)
		return false;

	IQuestBearerInterface* QuestBearerInterface = Cast<IQuestBearerInterface>(PlayerController);
	if (!QuestBearerInterface)
		return false;


	return QuestBearerInterface->GetQuestBearerComponent()->Authority_TryProgressQuest(QuestAsset->QuestID, this);
}
