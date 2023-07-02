import unreal


MAPS_PATH = '/Game/Core/Maps/'

def open(name: str):
    unreal.EditorLoadingAndSavingUtils.load_map(MAPS_PATH + name)


def open_form():
    open(name='ClientSwitchForm')


def open_playground():
    open(name='ClientsPlayground')
