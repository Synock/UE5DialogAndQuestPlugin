// Copyright 2022 Maximilien (Synock) Guislain


#include "Actors/AreaQuestValidator.h"

#include "Interfaces/QuestBearerInterface.h"
<<<<<<< HEAD
=======
#include "Misc/DialogAndQuestPluginHelper.h"
#include "Net/UnrealNetwork.h"
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84


// Sets default values
AAreaQuestValidator::AAreaQuestValidator(): AActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

<<<<<<< HEAD
	OnActorBeginOverlap.AddDynamic(this, &AAreaQuestValidator::OnOverlapBegin);
	OnActorEndOverlap.AddDynamic(this, &AAreaQuestValidator::OnOverlapEnd);

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	BoxComponent->SetBoxExtent(FVector(200.f, 200.f, 200.f));
	BoxComponent->bDynamicObstacle = true;
	BoxComponent->SetupAttachment(GetRootComponent());
	BoxComponent->SetGenerateOverlapEvents(true);

=======
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
	if (HasAuthority())
	{
		QuestComponent = CreateDefaultSubobject<UQuestGiverComponent>("QuestComponent");
		QuestComponent->SetIsReplicated(false); // Enable replication by default
<<<<<<< HEAD
=======

		BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
		BoxComponent->SetBoxExtent(FVector(200.f, 200.f, 200.f));
		BoxComponent->bDynamicObstacle = true;
		BoxComponent->SetupAttachment(GetRootComponent());
		BoxComponent->SetGenerateOverlapEvents(true);
		BoxComponent->SetIsReplicated(false);
		SetRootComponent(BoxComponent);

		OnActorBeginOverlap.AddDynamic(this, &AAreaQuestValidator::OnOverlapBegin);
		OnActorEndOverlap.AddDynamic(this, &AAreaQuestValidator::OnOverlapEnd);
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
	}
}

// Called when the game starts or when spawned
void AAreaQuestValidator::BeginPlay()
{
	Super::BeginPlay();

	DrawDebugBox(GetWorld(), GetActorLocation(), GetComponentsBoundingBox().GetExtent(), FColor::Red, true, -1, 0, 5);
}

void AAreaQuestValidator::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
<<<<<<< HEAD
	if(!HasAuthority())
		return;

	if(OtherActor && OtherActor != this)
	{
		if(IQuestBearerInterface* QuestBearer = Cast<IQuestBearerInterface>(OtherActor); QuestBearer != nullptr)
		{
			const auto & QuestData = QuestComponent->GetValidableQuestSteps();
			for(const auto & QData : QuestData)
			{
				QuestBearer->GetQuestBearerComponent()->TryValidateQuestFromActor(QData.Key, OtherActor);
			}
		}
	}
}

void AAreaQuestValidator::OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor)
{
	if(!HasAuthority())
		return;

	if(OtherActor && OtherActor != this)
	{

	}
}

=======
	UDialogAndQuestPluginHelper::Log(
		"Overlap begin between " + OverlappedActor->GetName() + " and " + OtherActor->GetName());
	APawn* ActorAsPawn = Cast<APawn>(OtherActor);
	if (!ActorAsPawn)
		return;

	APlayerController* PlayerController = Cast<APlayerController>(ActorAsPawn->GetController());
	if (!PlayerController)
		return;

	IQuestBearerInterface* QuestBearerInterface = Cast<IQuestBearerInterface>(PlayerController);
	if (!QuestBearerInterface)
		return;

	QuestBearerInterface->TryProgressAll(this);
}

void AAreaQuestValidator::OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor)
{
	UDialogAndQuestPluginHelper::Log(
		"Overlap end between " + OverlappedActor->GetName() + " and " + OtherActor->GetName());
}
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
