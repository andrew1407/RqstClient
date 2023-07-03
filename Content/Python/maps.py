import unreal


MAPS_PATH = '/Game/Core/Maps/'
CLIENT_SWITCH_FORM_MAP = 'ClientSwitchForm'
CLIENTS_PLAYGROUND_MAP = 'ClientsPlayground'


def open(name: str):
    unreal.EditorLoadingAndSavingUtils.load_map(MAPS_PATH + name)


def open_form():
    open(name=CLIENT_SWITCH_FORM_MAP)


def open_playground():
    open(name=CLIENTS_PLAYGROUND_MAP)


def switch():
    load_destinations = {
        CLIENT_SWITCH_FORM_MAP: CLIENTS_PLAYGROUND_MAP,
        CLIENTS_PLAYGROUND_MAP: CLIENT_SWITCH_FORM_MAP,
    }
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    name = world.get_name()
    if name in load_destinations: open(load_destinations[name])
