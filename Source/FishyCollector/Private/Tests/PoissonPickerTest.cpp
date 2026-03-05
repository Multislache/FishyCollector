// Fill out your copyright notice in the Description page of Project Settings.

#include "Misc/AutomationTest.h"
#include "FishyCollectorGameMode.h"
#include "PoissonTemplate.h"
#include "LieuTemplate.h"

#if WITH_DEV_AUTOMATION_TESTS

// ─── Helpers ────────────────────────────────────────────────────────────────

static UPoissonTemplate* CreateTestPoisson(EPoissonRarete Rarete)
{
	UPoissonTemplate* P = NewObject<UPoissonTemplate>(GetTransientPackage());
	P->Rarete = Rarete;
	return P;
}

static UDataTable* CreateTestTable(const FLieuRow& Row, FName RowName = "TestLieu")
{
	UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
	Table->RowStruct = FLieuRow::StaticStruct();
	Table->AddRow(RowName, Row);
	return Table;
}

// ─── Tests ──────────────────────────────────────────────────────────────────

// Table nulle → bSucces=false, résultat null
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPoissonPicker_TableNulle,
	"FishyCollector.PoissonPicker.TableNulle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FPoissonPicker_TableNulle::RunTest(const FString& Parameters)
{
	bool bSucces;
	UPoissonTemplate* Result = AFishyCollectorGameMode::TirerUnPoissonDepuisTable(nullptr, "TestLieu", bSucces);
	TestFalse("bSucces doit être false avec une table nulle", bSucces);
	TestNull("Le résultat doit être null avec une table nulle", Result);
	return true;
}

// Lieu inexistant → bSucces=false, résultat null
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPoissonPicker_LieuIntrouvable,
	"FishyCollector.PoissonPicker.LieuIntrouvable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FPoissonPicker_LieuIntrouvable::RunTest(const FString& Parameters)
{
	FLieuRow Row;
	Row.Poissons.Add(CreateTestPoisson(EPoissonRarete::Commun));
	UDataTable* Table = CreateTestTable(Row, "Lac");

	bool bSucces;
	UPoissonTemplate* Result = AFishyCollectorGameMode::TirerUnPoissonDepuisTable(Table, "LieuInexistant", bSucces);
	TestFalse("bSucces doit être false pour un lieu inexistant", bSucces);
	TestNull("Le résultat doit être null pour un lieu inexistant", Result);
	return true;
}

// Liste de poissons vide → bSucces=false, résultat null
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPoissonPicker_ListeVide,
	"FishyCollector.PoissonPicker.ListeVide",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FPoissonPicker_ListeVide::RunTest(const FString& Parameters)
{
	FLieuRow Row; // Poissons vide
	UDataTable* Table = CreateTestTable(Row);

	bool bSucces;
	UPoissonTemplate* Result = AFishyCollectorGameMode::TirerUnPoissonDepuisTable(Table, "TestLieu", bSucces);
	TestFalse("bSucces doit être false avec une liste vide", bSucces);
	TestNull("Le résultat doit être null avec une liste vide", Result);
	return true;
}

// Tirage normal → bSucces=true, résultat valide
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPoissonPicker_TirageReussi,
	"FishyCollector.PoissonPicker.TirageReussi",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FPoissonPicker_TirageReussi::RunTest(const FString& Parameters)
{
	FLieuRow Row;
	Row.Poissons.Add(CreateTestPoisson(EPoissonRarete::Commun));
	Row.Poissons.Add(CreateTestPoisson(EPoissonRarete::Rare));
	Row.Poissons.Add(CreateTestPoisson(EPoissonRarete::Legendaire));
	UDataTable* Table = CreateTestTable(Row);

	bool bSucces;
	UPoissonTemplate* Result = AFishyCollectorGameMode::TirerUnPoissonDepuisTable(Table, "TestLieu", bSucces);
	TestTrue("bSucces doit être true avec une liste valide", bSucces);
	TestNotNull("Le résultat ne doit pas être null", Result);
	return true;
}

// Fallback : la rareté tirée n'existe pas dans le lieu → retourne quand même un poisson
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPoissonPicker_Fallback,
	"FishyCollector.PoissonPicker.Fallback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FPoissonPicker_Fallback::RunTest(const FString& Parameters)
{
	FLieuRow Row;
	Row.ChanceCommun = 0.f;
	Row.ChanceRare = 0.f;
	Row.ChanceLegendaire = 100.f; // Force Légendaire
	// Mais le lieu ne contient que des Communs
	Row.Poissons.Add(CreateTestPoisson(EPoissonRarete::Commun));
	Row.Poissons.Add(CreateTestPoisson(EPoissonRarete::Commun));
	UDataTable* Table = CreateTestTable(Row);

	bool bSucces;
	UPoissonTemplate* Result = AFishyCollectorGameMode::TirerUnPoissonDepuisTable(Table, "TestLieu", bSucces);
	TestTrue("Le fallback doit retourner bSucces=true", bSucces);
	TestNotNull("Le fallback doit retourner un poisson valide", Result);
	return true;
}

// Distribution statistique sur 10 000 tirages (tolérance ±5%)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPoissonPicker_Distribution,
	"FishyCollector.PoissonPicker.Distribution",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FPoissonPicker_Distribution::RunTest(const FString& Parameters)
{
	FLieuRow Row;
	Row.ChanceCommun    = 70.f;
	Row.ChanceRare      = 25.f;
	Row.ChanceLegendaire = 5.f;

	UPoissonTemplate* PoissonCommun     = CreateTestPoisson(EPoissonRarete::Commun);
	UPoissonTemplate* PoissonRare       = CreateTestPoisson(EPoissonRarete::Rare);
	UPoissonTemplate* PoissonLegendaire = CreateTestPoisson(EPoissonRarete::Legendaire);

	Row.Poissons.Add(PoissonCommun);
	Row.Poissons.Add(PoissonRare);
	Row.Poissons.Add(PoissonLegendaire);

	UDataTable* Table = CreateTestTable(Row);

	constexpr int32 NbTirages  = 10000;
	constexpr float Tolerance  = 5.f; // ±5%

	int32 NbCommun = 0, NbRare = 0, NbLegendaire = 0;
	for (int32 i = 0; i < NbTirages; ++i)
	{
		bool bSucces;
		UPoissonTemplate* Result = AFishyCollectorGameMode::TirerUnPoissonDepuisTable(Table, "TestLieu", bSucces);
		if      (Result == PoissonCommun)     NbCommun++;
		else if (Result == PoissonRare)       NbRare++;
		else if (Result == PoissonLegendaire) NbLegendaire++;
	}

	const float PctCommun     = static_cast<float>(NbCommun)     / NbTirages * 100.f;
	const float PctRare       = static_cast<float>(NbRare)       / NbTirages * 100.f;
	const float PctLegendaire = static_cast<float>(NbLegendaire) / NbTirages * 100.f;

	AddInfo(FString::Printf(TEXT("Résultats sur %d tirages — Commun: %.1f%% | Rare: %.1f%% | Légendaire: %.1f%%"),
		NbTirages, PctCommun, PctRare, PctLegendaire));

	TestTrue(FString::Printf(TEXT("Commun devrait être ~70%% (obtenu %.1f%%)"), PctCommun),
		FMath::Abs(PctCommun - 70.f) <= Tolerance);
	TestTrue(FString::Printf(TEXT("Rare devrait être ~25%% (obtenu %.1f%%)"), PctRare),
		FMath::Abs(PctRare - 25.f) <= Tolerance);
	TestTrue(FString::Printf(TEXT("Légendaire devrait être ~5%% (obtenu %.1f%%)"), PctLegendaire),
		FMath::Abs(PctLegendaire - 5.f) <= Tolerance);

	return true;
}

// Distribution sur 10 000 tirages avec la vraie DataTable du projet
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPoissonPicker_VraieDataTable_Distribution,
	"FishyCollector.PoissonPicker.VraieDataTable.Distribution",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FPoissonPicker_VraieDataTable_Distribution::RunTest(const FString& Parameters)
{
	UDataTable* Table = LoadObject<UDataTable>(nullptr, TEXT("/Game/Blueprint/Lieu/Lieux.Lieux"));
	TestNotNull("La DataTable Lieux doit être chargeable", Table);
	if (!Table)
		return false;

	constexpr int32 NbTirages = 10000;

	for (const FName& RowName : Table->GetRowNames())
	{
		TMap<FString, int32> Compteurs;
		Compteurs.Add("Commun",     0);
		Compteurs.Add("Rare",       0);
		Compteurs.Add("Legendaire", 0);
		Compteurs.Add("Echec",      0);

		for (int32 i = 0; i < NbTirages; ++i)
		{
			bool bSucces;
			UPoissonTemplate* Result = AFishyCollectorGameMode::TirerUnPoissonDepuisTable(Table, RowName, bSucces);
			if (!bSucces || !Result)
			{
				Compteurs["Echec"]++;
				continue;
			}
			switch (Result->Rarete)
			{
				case EPoissonRarete::Commun:     Compteurs["Commun"]++;     break;
				case EPoissonRarete::Rare:       Compteurs["Rare"]++;       break;
				case EPoissonRarete::Legendaire: Compteurs["Legendaire"]++; break;
			}
		}

		AddInfo(FString::Printf(
			TEXT("[%s] sur %d tirages — Commun: %.1f%% | Rare: %.1f%% | Légendaire: %.1f%% | Échec: %d"),
			*RowName.ToString(), NbTirages,
			Compteurs["Commun"]     / (float)NbTirages * 100.f,
			Compteurs["Rare"]       / (float)NbTirages * 100.f,
			Compteurs["Legendaire"] / (float)NbTirages * 100.f,
			Compteurs["Echec"]
		));
	}

	return true;
}

// 100 tirages sur NewRow avec statistiques par poisson
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPoissonPicker_NewRow_Statistiques,
	"FishyCollector.PoissonPicker.VraieDataTable.NewRow.Statistiques",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FPoissonPicker_NewRow_Statistiques::RunTest(const FString& Parameters)
{
	UDataTable* Table = LoadObject<UDataTable>(nullptr, TEXT("/Game/Blueprint/Lieu/Lieux.Lieux"));
	TestNotNull("La DataTable Lieux doit être chargeable", Table);
	if (!Table)
		return false;

	constexpr int32 NbTirages = 100;
	TMap<FString, int32> CompteurParPoisson;
	TMap<FString, EPoissonRarete> RareteParPoisson;

	for (int32 i = 0; i < NbTirages; ++i)
	{
		bool bSucces;
		UPoissonTemplate* Result = AFishyCollectorGameMode::TirerUnPoissonDepuisTable(Table, "NewRow", bSucces);
		if (!bSucces || !Result)
		{
			CompteurParPoisson.FindOrAdd("ÉCHEC")++;
			continue;
		}
		const FString Nom = Result->Nom.ToString();
		CompteurParPoisson.FindOrAdd(Nom)++;
		RareteParPoisson.Add(Nom, Result->Rarete);
	}

	// Trier par nombre de tirages décroissant
	CompteurParPoisson.ValueSort([](int32 A, int32 B) { return A > B; });

	AddInfo(FString::Printf(TEXT("=== Statistiques NewRow sur %d tirages ==="), NbTirages));
	for (const auto& Pair : CompteurParPoisson)
	{
		const EPoissonRarete* Rarete = RareteParPoisson.Find(Pair.Key);
		FString RareteStr = "?";
		if (Rarete)
		{
			switch (*Rarete)
			{
				case EPoissonRarete::Commun:     RareteStr = "Commun";     break;
				case EPoissonRarete::Rare:       RareteStr = "Rare";       break;
				case EPoissonRarete::Legendaire: RareteStr = "Légendaire"; break;
			}
		}
		AddInfo(FString::Printf(TEXT("  %-20s [%-10s] : %3d fois (%.1f%%)"),
			*Pair.Key, *RareteStr, Pair.Value, Pair.Value / (float)NbTirages * 100.f));
	}

	return true;
}

// Test sur la vraie DataTable du projet
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPoissonPicker_VraieDataTable,
	"FishyCollector.PoissonPicker.VraieDataTable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FPoissonPicker_VraieDataTable::RunTest(const FString& Parameters)
{
	UDataTable* Table = LoadObject<UDataTable>(nullptr, TEXT("/Game/Blueprint/Lieu/Lieux.Lieux"));
	TestNotNull("La DataTable Lieux doit être chargeable", Table);
	if (!Table)
		return false;

	// Tire un poisson depuis chaque ligne de la table
	TArray<FName> RowNames = Table->GetRowNames();
	TestTrue("La DataTable doit contenir au moins une ligne", RowNames.Num() > 0);

	for (const FName& RowName : RowNames)
	{
		bool bSucces;
		UPoissonTemplate* Result = AFishyCollectorGameMode::TirerUnPoissonDepuisTable(Table, RowName, bSucces);

		AddInfo(FString::Printf(TEXT("Lieu '%s' → %s"),
			*RowName.ToString(),
			bSucces ? *Result->Nom.ToString() : TEXT("ÉCHEC")));

		TestTrue(FString::Printf(TEXT("Le tirage depuis '%s' doit réussir"), *RowName.ToString()), bSucces);
		TestNotNull(FString::Printf(TEXT("Le résultat depuis '%s' ne doit pas être null"), *RowName.ToString()), Result);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
