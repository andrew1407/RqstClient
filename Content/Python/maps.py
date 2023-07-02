import unreal


MAPS_PATH = '/Game/Core/Maps/'

def open(name: str):
    unreal.EditorLoadingAndSavingUtils.load_map(MAPS_PATH + name)


def open_form():
    open(name='ClientSwitchForm')


def open_playground():
    open(name='ClientsPlayground')


def switch():
    load_destinations = {
        'ClientSwitchForm': 'ClientsPlayground',
        'ClientsPlayground': 'ClientSwitchForm',
    }
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    name = world.get_name()
    if name in load_destinations: open(load_destinations[name])
