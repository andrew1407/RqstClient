from typing import Optional, Iterable, Union
import unreal


OUTLINER_FOLDER = 'ClientStrategies'
ACTOR_CLASS_NAME = 'BP_StrategyActivator'
STRATEGY_ACTOR_PATH = '/Game/Core/Blueprints/Gameplay/' + ACTOR_CLASS_NAME
PROPS_ASSET_PATH = '/Plugins/ClientConnectionStrategies/DataAssets/DA_ClientCollection'

HOST = 'localhost'
PORT = 8080


def get_client_labels() -> dict[str, unreal.ClientLabels]:
    keys = 'NONE', 'HTTP', 'WS', 'UDP', 'TCP'
    return { k: getattr(unreal.ClientLabels, k) for k in keys }


def spawn_activators(start: unreal.Vector, step: unreal.Vector, labels: Optional[tuple[str]] = None):
    client_labels = get_client_labels()
    labels_to_spawn = None
    if labels is None:
        labels_to_spawn = tuple(client_labels.keys())
    else:
        labels_to_spawn = tuple(l.upper() for l in labels if l.upper() in client_labels.keys())
    i = 0
    position_vec = start.copy()
    actor_class = unreal.EditorAssetLibrary.load_blueprint_class(STRATEGY_ACTOR_PATH)
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
            actor.set_editor_property('StrategyType', enum_value)
            position_vec += step
            if enum_value == unreal.ClientLabels.NONE: continue
            actor.set_editor_properties(dict(Host=HOST, Port=PORT + i))
            i += 1


def select_from(search: Union[str, Iterable[str]]):
    client_labels = get_client_labels()
    if isinstance(search, str): search = (search, )
    labels = tuple(client_labels[l] for l in set(s.upper() for s in search) if l in client_labels)
    if not labels: return
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    actor_class = unreal.EditorAssetLibrary.load_blueprint_class(STRATEGY_ACTOR_PATH)
    actors = unreal.GameplayStatics.get_all_actors_of_class(world, actor_class)
    for actor in actors:
        strategy_type: unreal.ClientLabels = actor.get_editor_property('StrategyType')
        if strategy_type in labels:
            unreal.EditorLevelLibrary.set_actor_selection_state(actor, should_be_selected=True)
