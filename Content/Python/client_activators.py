from typing import Optional, Iterable, Union
import unreal


OUTLINER_FOLDER = 'ClientStrategies'
ACTOR_CLASS_NAME = 'BP_StrategyActivator'
STRATEGY_ACTOR_PATH = '/Game/Core/Blueprints/Gameplay/' + ACTOR_CLASS_NAME

DATA_ASSET_DIR = '/ClientConnectionStrategies/DataAssets/'
PROPS_ASSET_PATH = DATA_ASSET_DIR + 'DA_DefaultFieldsFilling'
CLIENT_COLLECTION_ASSET_PATH = DATA_ASSET_DIR + 'DA_ClientCollection'


ActorsArr = unreal.Array[unreal.Actor]
ColorCollection = unreal.Map[unreal.ClientLabels, unreal.Color]
ActivatorLabels = Optional[Iterable[str]]
ActivatorsToSearch = Union[str, Iterable[str]]

def get_client_labels() -> dict[str, unreal.ClientLabels]:
    keys = 'NONE', 'HTTP', 'WS', 'UDP', 'TCP'
    return { k: getattr(unreal.ClientLabels, k) for k in keys }


def load_props_asset() -> unreal.ClientCollection:
    return unreal.EditorAssetLibrary.load_asset(PROPS_ASSET_PATH)


def load_strategies_asset() -> unreal.ClientCollection:
    return unreal.EditorAssetLibrary.load_asset(CLIENT_COLLECTION_ASSET_PATH)


def spawn_activators(start: unreal.Vector, step: unreal.Vector, labels: ActivatorLabels = None) -> ActorsArr:
    client_labels = get_client_labels()
    labels_to_spawn = None
    if labels is None:
        labels_to_spawn = tuple(client_labels.keys())
    else:
        labels_to_spawn = tuple(l.upper() for l in labels if l.upper() in client_labels.keys())
    i = 0
    position_vec = start.copy()
    props = load_props_asset()
    styles = load_strategies_asset()
    client_colors: ColorCollection = styles.get_editor_property('ClientColors')
    get_color = lambda val: client_colors.get(key=val, default=styles.get_editor_property('DefaultColor'))
    actor_class = unreal.EditorAssetLibrary.load_blueprint_class(STRATEGY_ACTOR_PATH)
    spawned: ActorsArr = unreal.Array(type=unreal.Actor)
    loading_title = 'Spawning client strategy activators: '
    with unreal.ScopedSlowTask(work=len(labels_to_spawn), desc=loading_title + '...') as slow_task:
        slow_task.make_dialog(can_cancel=True)
        for label in labels_to_spawn:
            if slow_task.should_cancel(): break
            slow_task.enter_progress_frame(work=1, desc=loading_title + label)
            enum_value = client_labels[label]
            actor = unreal.EditorLevelLibrary().spawn_actor_from_class(actor_class, position_vec)
            actor.set_actor_label(f'{ACTOR_CLASS_NAME}_{label}')
            actor.set_folder_path(OUTLINER_FOLDER)
            color = get_color(enum_value)
            actor.set_editor_properties(dict(
                StrategyType=enum_value,
                Color=unreal.LinearColor(color.r, color.g, color.b),
            ))
            position_vec += step
            spawned.append(actor)
            if enum_value == unreal.ClientLabels.NONE: continue
            actor.set_editor_properties(dict(
                Host=props.get_editor_property('Host'),
                Port=props.get_editor_property('Port') + i,
            ))
            i += 1
    return spawned


def select_from(search: ActivatorsToSearch) -> ActorsArr:
    client_labels = get_client_labels()
    if isinstance(search, str): search = (search, )
    labels = tuple(client_labels[l] for l in set(s.upper() for s in search) if l in client_labels)
    if not labels: return
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actor_class = unreal.EditorAssetLibrary.load_blueprint_class(STRATEGY_ACTOR_PATH)
    actors = unreal.GameplayStatics.get_all_actors_of_class(world, actor_class)
    selected: ActorsArr = unreal.Array(type=unreal.Actor)
    for actor in actors:
        strategy_type: unreal.ClientLabels = actor.get_editor_property('StrategyType')
        if strategy_type not in labels: continue
        actor_subsystem.set_actor_selection_state(actor, should_be_selected=True)
        selected.append(actor)
    return selected
    
