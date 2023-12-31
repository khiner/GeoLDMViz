#pragma once

struct Window {
    const char *Name{""};
    bool Visible{true};
};

struct WindowsState {
    Window SceneControls{"Scene controls"};
    Window Scene{"Scene"};
    Window MoleculeChainControls{"Molecule chain"};
    // By default, the demo window is docked, but not visible.
    Window ImGuiDemo{"Dear ImGui demo", false};
};
