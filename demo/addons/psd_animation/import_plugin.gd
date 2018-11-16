# import_plugin.gd
tool
extends EditorImportPlugin

var PsdImporterClass = preload('res://addons/psd_animation/psd_importer.gdns')

enum Presets { PRESET_DEFAULT }

func get_importer_name():
	return "stoneveil.psdanimation"

func get_visible_name():
	return "PSD Animation Importer"

func get_recognized_extensions():
	return ["psd"]

func get_save_extension():
	return "tres"

func get_resource_type():
	return "SpriteFrames"

func get_preset_count():
	return Presets.size()

func get_preset_name(preset):
	match preset:
		Presets.PRESET_DEFAULT:
			return "Default"
		_:
			return "Unknown"

func get_import_options(preset):
	match preset:
		Presets.PRESET_DEFAULT:
			return [{
				"name": "just_extract_layers",
				"default_value": false
				}]
		_:
			return []

func get_option_visibility(option, options):
	return true

func import(source_file, save_path, options, r_platform_variants, r_gen_files):
	var file = File.new()
	var err = file.open(source_file, File.READ)
	if err != OK:
		return err

	var real_path = file.get_path_absolute()
	var path = file.get_path()
	file.close()

	var dir = path.get_base_dir()
	if dir.begins_with('res://'):
		dir = dir.right('res://'.length())
	
	var PsdImporter = PsdImporterClass.new()

	var success = PsdImporter.file_load(real_path)
	if !success:
		return false

	success = PsdImporter.extract_psd(dir)
	if options.just_extract_layers:
		return success

	if not PsdImporter.is_sprite_frames():
		return false
	print('is sprite frames')

	var animations = PsdImporter.get_sprite_frame_names()

	var sprframes = SpriteFrames.new()
	if sprframes.has_animation('default'):
		sprframes.remove_animation('default')
	for anim in animations.keys():
		print('animation: ', anim)
		sprframes.add_animation(anim)
		for frame in animations[anim]:
			var filename = dir + '/' + anim + '/' + frame + '.png'
			var texture = load(filename)
			sprframes.add_frame(anim, texture)
#		print(ResourceSaver.get_recognized_extensions(sprframes))
	return ResourceSaver.save("%s.%s" % [save_path, get_save_extension()], sprframes)
