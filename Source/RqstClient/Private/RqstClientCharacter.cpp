// Copyright Epic Games, Inc. All Rights Reserved.

#include "RqstClientCharacter.h"
#include "Camera/CameraComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "ClientHandler/Interactive.h"

//////////////////////////////////////////////////////////////////////////
// ARqstClientCharacter

ARqstClientCharacter::ARqstClientCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ARqstClientCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (IsValid(Subsystem)) Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	MapByMaterialIndices([this] (uint8 Id)
	{
		UMaterialInterface* Material = GetMesh()->GetMaterial(Id);
		UMaterialInstanceDynamic* Dynamic = UMaterialInstanceDynamic::Create(Material, nullptr);
		GetMesh()->SetMaterial(Id, Dynamic);
	});
}

//////////////////////////////////////////////////////////////////////////
// Input

void ARqstClientCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	if (!IsValid(EnhancedInputComponent)) return;
		
	//Jumping
	if (IsValid(JumpAction))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}

	//Moving
	if (IsValid(MoveAction)) EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARqstClientCharacter::Move);

	//Looking
	if (IsValid(LookAction)) EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ARqstClientCharacter::Look);

	if (IsValid(InteractAction)) EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ARqstClientCharacter::Interact);
}

void ARqstClientCharacter::Move(const FInputActionValue& Value)
{
	if (!IsValid(Controller)) return;

	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// find out which way is forward
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get forward vector
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	// get right vector 
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// add movement 
	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ARqstClientCharacter::Look(const FInputActionValue& Value)
{
	if (!IsValid(Controller)) return;

	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// add yaw and pitch input to controller
	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void ARqstClientCharacter::SetMaterialColor(const FLinearColor& Color)
{
	MapByMaterialIndices([&] (uint8 Id)
	{
		UMaterialInterface* MaterialInterface = GetMesh()->GetMaterial(Id);
		UMaterialInstanceDynamic* Material = Cast<UMaterialInstanceDynamic>(MaterialInterface);
		if (IsValid(Material)) Material->SetVectorParameterValue("Tint", Color);
	});
}

void ARqstClientCharacter::Interact()
{
	TArray<AActor*> OverlappingActors;
	GetCapsuleComponent()->GetOverlappingActors(OverlappingActors);
	AActor* ActorToInteract = nullptr;
	for (auto& Actor : OverlappingActors)
	{
		bool IsInteractive = UKismetSystemLibrary::DoesImplementInterface(Actor, UInteractive::StaticClass());
		if (!IsInteractive) continue;
		ActorToInteract = Actor;
		break;
	}
	if (IsValid(ActorToInteract)) IInteractive::Execute_Interract(ActorToInteract);
}

void ARqstClientCharacter::MapByMaterialIndices(const TFunctionRef<void(uint8)> Mapper)
{
	const uint8 Indices[] = { 0, 1 };
	for (const auto& Id : Indices) Mapper(Id);
}
