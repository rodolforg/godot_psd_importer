tool
extends EditorPlugin

var import_plugin

func _enter_tree():
	var PsdImporter = preload('res://addons/psd_animation/psd_importer.gdns').new()
	if PsdImporter.get_version() != 2:
		printerr('PsdImporter addon shared library version mismatch')
		return
	import_plugin = preload("import_plugin.gd").new()
	add_import_plugin(import_plugin)

func _exit_tree():
	if not import_plugin:
		return
	remove_import_plugin(import_plugin)
	import_plugin = null
