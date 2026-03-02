#include "HUD/SlashOverly.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void USlashOverly::SetHealthBarPercent(float Percent)
{
	if (HealthProgressBar)
	{
		HealthProgressBar->SetPercent(Percent);
	}
}

void USlashOverly::SetStaminaBarPercent(float Percent)
{
	if (StaminaProgressBar)
	{
		StaminaProgressBar->SetPercent(Percent);
	}
}

void USlashOverly::SetGold(int32 Gold)
{
	GoldConutText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Gold)));
}

void USlashOverly::SetSouls(int32 Souls)
{
	SoulConutText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Souls)));
}
