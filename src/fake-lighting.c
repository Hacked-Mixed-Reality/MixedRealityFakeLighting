/*
Mixed Reality Fake Lighting
Copyright (C) 2023 Hacked Mixed Reality hackedmixedreality@gmail.com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <fake-lighting.h>

#include "plugin-macros.generated.h"

struct fake_lighting_data {
	obs_source_t *target;
	obs_source_t *light_source;
	gs_effect_t *effect;

	gs_texture_t *light_source_tex;
	gs_texrender_t *texrender;

	//Storage for the parameter values
	float saturation;
	float intensity;
	float contrast;
	int partitions;
	int sub_divisions;
	const char *light_source_name;

	//Pointers to the Shader Parameters
	gs_eparam_t *light_source_param;
	gs_eparam_t *saturation_param;
	gs_eparam_t *intentsity_param;
	gs_eparam_t *contrast_param;
	gs_eparam_t *partitions_param;
	gs_eparam_t *sub_divisions_param;
};

const char *fake_lighting_get_name(void *data)
{
	UNUSED_PARAMETER(data);
	return "Mixed Reality Fake Lighting";
}

void fake_lighting_update(void *data, obs_data_t *settings)
{
	struct fake_lighting_data *filter_data = data;

	const char *source_name = obs_data_get_string(settings, "source");
	filter_data->light_source = obs_get_source_by_name(source_name);

	filter_data->saturation =
		(float)obs_data_get_double(settings, "saturation");
	filter_data->intensity =
		(float)obs_data_get_double(settings, "intensity");
	filter_data->contrast =
		(float)obs_data_get_double(settings, "contrast");
	filter_data->partitions = (int)obs_data_get_int(settings, "partitions");
	filter_data->sub_divisions =
		(int)obs_data_get_int(settings, "sub_divisions");
	filter_data->light_source_name = source_name;
	//blog(LOG_INFO, "Source changed to: %s", source_name);
}

void *fake_lighting_create(obs_data_t *settings, obs_source_t *source)
{
	//UNUSED_PARAMETER(settings);
	struct fake_lighting_data *filter_data =
		(struct fake_lighting_data *)bzalloc(
			sizeof(struct fake_lighting_data));

	// Retrieve any configuration settings from 'settings'
	//and initialize the filter data
	fake_lighting_update(filter_data, settings);
	blog(LOG_INFO, "Creating Plugin");

	filter_data->target = source;

	//effect file is found in the data folder
	char *filename = obs_module_file("fake_lighting.effect");

	//Need to "enter" graphics to load the shader
	obs_enter_graphics();
	gs_effect_t *effect = gs_effect_create_from_file(filename, NULL);

	if (!effect) {
		blog(LOG_WARNING,
		     "[obs-mixed-reality-fake-lighting] Shader not loaded");
	} else {
		filter_data->effect = effect;
	}

	//Go get the parameter hooks from the shader
	filter_data->intentsity_param =
		gs_effect_get_param_by_name(effect, "intensity");
	filter_data->saturation_param =
		gs_effect_get_param_by_name(effect, "saturation");
	filter_data->light_source_param =
		gs_effect_get_param_by_name(effect, "light_source");
	filter_data->contrast_param =
		gs_effect_get_param_by_name(effect, "contrast");
	filter_data->partitions_param =
		gs_effect_get_param_by_name(effect, "partitions");
	filter_data->sub_divisions_param =
		gs_effect_get_param_by_name(effect, "sub_divisions");

	obs_leave_graphics();

	return filter_data;
}

void fake_lighting_destroy(void *data)
{
	struct fake_lighting_data *filter_data =
		(struct fake_lighting_data *)data;

	// Cleanup and deallocate any resources used by the filter data

	bfree(filter_data);
}

void fake_lighting_defaults(obs_data_t *settings)
{
	blog(LOG_INFO, "Setting all default values");
	obs_data_set_default_double(settings, "saturation",
				    SATURATION_DEFAULT_VALUE);
	obs_data_set_default_double(settings, "intensity",
				    INTENSITY_DEFAULT_VALUE);
	obs_data_set_default_double(settings, "contrast",
				    CONTRAST_DEFAULT_VALUE);
	obs_data_set_default_int(settings, "partitions",
				 PARTITIONS_DEFAULT_VALUE);
	obs_data_set_default_int(settings, "sub_divisions",
				 SUB_DIVISIONS_DEFAULT_VALUE);
}

bool source_list_add_source(void *data, obs_source_t *source)
{
	obs_property_t *prop = data;

	const char *name = obs_source_get_name(source);
	size_t count = obs_property_list_item_count(prop);
	size_t idx = 0;
	while (idx < count &&
	       strcmp(name, obs_property_list_item_string(prop, idx)) > 0)
		idx++;
	obs_property_list_insert_string(prop, idx, name, name);
	return true;
}

obs_properties_t *fake_lighting_properties(void *data)
{
	UNUSED_PARAMETER(data);
	obs_properties_t *props = obs_properties_create();

	obs_property_t *p = obs_properties_add_list(props, "source",
						    obs_module_text("Source"),
						    OBS_COMBO_TYPE_EDITABLE,
						    OBS_COMBO_FORMAT_STRING);
	obs_enum_sources(source_list_add_source, p);
	obs_enum_scenes(source_list_add_source, p);
	obs_property_list_insert_string(p, 0, "", "");

	obs_properties_add_int_slider(props, "partitions",
				      obs_module_text("Grid Size"),
				      PARTITIONS_MIN_VALUE,
				      PARTITIONS_MAX_VALUE,
				      PARTITIONS_STEP_VALUE);

	obs_properties_add_int_slider(props, "sub_divisions",
				      obs_module_text("Smoothness"),
				      SUB_DIVISIONS_MIN_VALUE,
				      SUB_DIVISIONS_MAX_VALUE,
				      SUB_DIVISIONS_STEP_VALUE);

	obs_properties_add_float_slider(props, "saturation",
					obs_module_text("Saturation"),
					SATURATION_MIN_VALUE,
					SATURATION_MAX_VALUE,
					SATURATION_STEP_VALUE);

	obs_properties_add_float_slider(
		props, "intensity", obs_module_text("Intensity"),
		INTENSITY_MIN_VALUE, INTENSITY_MAX_VALUE, INTENSITY_STEP_VALUE);

	obs_properties_add_float_slider(props, "contrast",
					obs_module_text("Contrast"),
					CONTRAST_MIN_VALUE, CONTRAST_MAX_VALUE,
					CONTRAST_STEP_VALUE);
	return props;
}

void fake_lighting_render(void *data, gs_effect_t *effect)
{
	UNUSED_PARAMETER(effect);
	//Get the data from the data structure outlined above.
	struct fake_lighting_data *filter_data =
		(struct fake_lighting_data *)data;

	obs_source_t *target = filter_data->target;

	if (!filter_data->effect) {
		obs_source_skip_video_filter(target);
		return;
	}

	int target_width = obs_source_get_base_width(target);
	int target_height = obs_source_get_base_height(target);

	//generate the texture for the light source

	gs_texrender_t *texrender = filter_data->texrender;
	if (texrender) {
		gs_texrender_reset(texrender);
	} else {
		blog(LOG_INFO, "Creating texture");
		texrender = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
		filter_data->texrender = texrender;
	}

	if (!gs_texrender_begin(texrender, target_width, target_height)) {
		return;
	}

	//Fix race condition at startup where plugin loads before scene
	//There's probably a much better place to fix this than the
	//renderloop, but that's my naievity.
	if (!filter_data->light_source) {
		if ((strcmp(filter_data->light_source_name, "") != 0)) {
			blog(LOG_ERROR, "Light source not loaded - Reloading");
			filter_data->light_source = obs_get_source_by_name(
				filter_data->light_source_name);
		}
	}

	obs_source_video_render(filter_data->light_source);
	gs_texrender_end(texrender);

	if (!texrender) {
		//obs_source_skip_video_filter(filter_data->target);
		blog(LOG_INFO, "Not Rendering");
		//return;
	}
	gs_texture_t *light = gs_texrender_get_texture(texrender);

	//Begin rendering. Exit if it fails.
	if (!obs_source_process_filter_begin(filter_data->target, GS_RGBA,
					     OBS_ALLOW_DIRECT_RENDERING)) {
		blog(LOG_ERROR, "RENDERING FAILED");
		return;
	}

	//Store the current blend state before using our own
	gs_blend_state_push();
	gs_blend_function(GS_BLEND_ONE, GS_BLEND_INVSRCALPHA);

	//Load in the extra data that the shader is going to use
	gs_effect_set_float(filter_data->intentsity_param,
			    filter_data->intensity);
	gs_effect_set_float(filter_data->saturation_param,
			    filter_data->saturation);
	gs_effect_set_texture(filter_data->light_source_param, light);
	gs_effect_set_float(filter_data->contrast_param, filter_data->contrast);
	gs_effect_set_int(filter_data->partitions_param,
			  filter_data->partitions);
	gs_effect_set_int(filter_data->sub_divisions_param,
			  filter_data->sub_divisions);

	//Run the effect (shader) across the full frame
	obs_source_process_filter_end(filter_data->target, filter_data->effect,
				      0, 0);

	//Retrieve the previous render state
	gs_blend_state_pop();
}

struct obs_source_info fake_lighting_video = {
	.id = "fake_lighting",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_VIDEO,
	.get_name = fake_lighting_get_name,
	.create = fake_lighting_create,
	.destroy = fake_lighting_destroy,
	.get_defaults = fake_lighting_defaults,
	.get_properties = fake_lighting_properties,
	.video_render = fake_lighting_render,
	.update = fake_lighting_update,
};

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

bool obs_module_load(void)
{
	obs_register_source(&fake_lighting_video);
	blog(LOG_INFO, "Fake Lighting loaded successfully (version %s)",
	     PLUGIN_VERSION);
	return true;
}

void obs_module_unload()
{
	blog(LOG_INFO, "plugin unloaded");
}
