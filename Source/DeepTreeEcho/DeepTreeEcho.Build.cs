// Copyright Deep Tree Echo. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class DeepTreeEcho : ModuleRules
{
	public DeepTreeEcho(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// Deep Tree Echo source directories (relative to repository root)
		string DeepTreeEchoRoot = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../DeepTreeEcho"));
		string ReservoirEchoRoot = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ReservoirEcho"));

		// Include paths for Deep Tree Echo components
		PublicIncludePaths.AddRange(
			new string[] {
				Path.Combine(DeepTreeEchoRoot, "Core"),
				Path.Combine(DeepTreeEchoRoot, "Reservoir"),
				Path.Combine(DeepTreeEchoRoot, "4ECognition"),
				Path.Combine(DeepTreeEchoRoot, "ActiveInference"),
				Path.Combine(DeepTreeEchoRoot, "Avatar"),
				Path.Combine(DeepTreeEchoRoot, "Cognitive"),
				Path.Combine(DeepTreeEchoRoot, "Cosmos"),
				Path.Combine(DeepTreeEchoRoot, "Entelechy"),
				Path.Combine(DeepTreeEchoRoot, "Evolution"),
				Path.Combine(DeepTreeEchoRoot, "Goals"),
				Path.Combine(DeepTreeEchoRoot, "Integration"),
				Path.Combine(DeepTreeEchoRoot, "Memory"),
				Path.Combine(DeepTreeEchoRoot, "Metamodel"),
				Path.Combine(DeepTreeEchoRoot, "Sensorimotor"),
				Path.Combine(DeepTreeEchoRoot, "System5"),
				Path.Combine(DeepTreeEchoRoot, "UnrealBridge"),
				Path.Combine(DeepTreeEchoRoot, "Wisdom"),
				// Sys6 Operad Architecture
				Path.Combine(DeepTreeEchoRoot, "Sys6"),
				// Taskflow parallel task scheduling
				Path.Combine(DeepTreeEchoRoot, "Taskflow"),

				// ---------------------------------------------------------------------------
				// These directories existed on disk with no include path registered, so their
				// headers were not even resolvable from this module.
				//
				// IMPORTANT AND UNRESOLVED: adding an include path does NOT put a .cpp file in
				// the build. UBT compiles translation units found under the MODULE directory
				// (Source/DeepTreeEcho/, currently 11 .cpp files). The root DeepTreeEcho/ tree
				// holds ~107 .cpp files that UBT has never compiled and still does not compile.
				// These entries fix header resolution only.
				//
				// That gap explains how a merge from main could break GameTraining (header kept
				// a bitmask refactor while the .cpp was rewritten against pre-refactor field
				// names) with nothing catching it: the only thing exercising that code was a
				// standalone g++ harness running MIRRORED logic, never the real translation
				// units. Roughly 107 implementation files are in the same position.
				//
				// Properly fixing this means either moving the implementations under
				// Source/DeepTreeEcho/, or declaring the root tree as an additional module with
				// its own Build.cs. Both are structural changes that need a real UBT run to
				// validate, and UBT cannot run in the environment this was written in (no
				// UnrealBuildTool binaries, no dotnet). Doing it blind would be worse than
				// leaving it documented.
				// ---------------------------------------------------------------------------
				Path.Combine(DeepTreeEchoRoot, "Learning"),
				Path.Combine(DeepTreeEchoRoot, "Embodied"),
				Path.Combine(DeepTreeEchoRoot, "GameTraining"),
				Path.Combine(DeepTreeEchoRoot, "Mastery"),
				Path.Combine(DeepTreeEchoRoot, "Fractal"),
			}
		);

		// Include paths for ReservoirCpp library (Eigen-based ESN)
		PrivateIncludePaths.AddRange(
			new string[] {
				Path.Combine(ReservoirEchoRoot, "reservoircpp_cpp/include"),
				Path.Combine(ReservoirEchoRoot, "external/eigen-3.4.0"),
				// Taskflow header-only library for parallel task graphs
				Path.Combine(ReservoirEchoRoot, "external/taskflow-3.8.0"),
			}
		);

		// Core Unreal Engine dependencies
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
				"SlateCore",
			}
		);

		// Enable exceptions for Eigen library
		bEnableExceptions = true;

		// Disable unity builds for better debugging
		bUseUnity = false;

		// Compiler definitions
		PublicDefinitions.Add("DEEPTREEECHO_API=");

		// UNREALECHO_API is normally defined by the UnrealEcho module. Several headers now
		// compiled here still use it (Memory/, Wisdom/, Attention/, Introspection/, Blueprint/),
		// and a header cannot see a definition from a module it does not depend on - so without
		// this the macro is simply undefined and every class using it fails to parse.
		//
		// Defined empty to match DEEPTREEECHO_API. Both are empty because this module is
		// statically linked into the target rather than exported as a DLL; if that ever changes,
		// both need real DLLEXPORT/DLLIMPORT handling, not empty strings.
		PublicDefinitions.Add("UNREALECHO_API=");

		PublicDefinitions.Add("EIGEN_MPL2_ONLY");  // Use only MPL2-licensed parts of Eigen
	}
}
