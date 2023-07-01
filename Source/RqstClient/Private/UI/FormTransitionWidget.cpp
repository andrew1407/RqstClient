// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/FormTransitionWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UFormTransitionWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetupTransitionBtnProperties();
}

void UFormTransitionWidget::GoToScene()
{
    if (LevelToOpen.IsNull()) return;
    SceneTransitionBtn->SetIsEnabled(false);
    UGameplayStatics::OpenLevel(GetWorld(), FName(LevelToOpen.GetAssetName()));
}

void UFormTransitionWidget::SetupTransitionBtnProperties()
{
    if (!IsValid(SceneTransitionBtn)) return;
    if (LevelToOpen.IsNull()) return SceneTransitionBtn->SetIsEnabled(false);
    SceneTransitionBtn->OnClicked.AddDynamic(this, &UFormTransitionWidget::GoToScene);
    UTextBlock* TextBlock = Cast<UTextBlock>(SceneTransitionBtn->GetChildAt(0));
    if (!IsValid(SceneTransitionBtn)) return;
    const FString Description = FString::Printf(TEXT("Go to \"%s\" level"), *LevelToOpen.GetAssetName());
    TextBlock->SetText(FText::FromString(Description));
}
