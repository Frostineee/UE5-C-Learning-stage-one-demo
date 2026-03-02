// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SlashHUD.h"
#include "HUD/SlashOverly.h"

void ASlashHUD::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* Controller = World->GetFirstPlayerController();
		if (Controller && SlashOverlyClass)
		{
			SlashOverly = CreateWidget<USlashOverly>(Controller, SlashOverlyClass);
			SlashOverly->AddToViewport();
		}
	}
}
