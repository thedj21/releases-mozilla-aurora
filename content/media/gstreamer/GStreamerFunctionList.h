/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __APPLE__

/*
 * List of symbol names we need to dlsym from the gstreamer library.
 */
GST_FUNC(LIBGSTAPP, gst_app_sink_get_type)
GST_FUNC(LIBGSTAPP, gst_app_sink_pull_buffer)
GST_FUNC(LIBGSTAPP, gst_app_sink_set_callbacks)
GST_FUNC(LIBGSTAPP, gst_app_src_end_of_stream)
GST_FUNC(LIBGSTAPP, gst_app_src_get_size)
GST_FUNC(LIBGSTAPP, gst_app_src_get_type)
GST_FUNC(LIBGSTAPP, gst_app_src_push_buffer)
GST_FUNC(LIBGSTAPP, gst_app_src_set_callbacks)
GST_FUNC(LIBGSTAPP, gst_app_src_set_size)
GST_FUNC(LIBGSTAPP, gst_app_src_set_stream_type)
GST_FUNC(LIBGSTREAMER, gst_bin_get_by_name)
GST_FUNC(LIBGSTREAMER, gst_bin_get_type)
GST_FUNC(LIBGSTREAMER, gst_bin_iterate_recurse)
GST_FUNC(LIBGSTREAMER, gst_buffer_copy_metadata)
GST_FUNC(LIBGSTREAMER, gst_buffer_get_qdata)
GST_FUNC(LIBGSTREAMER, gst_buffer_get_type)
GST_FUNC(LIBGSTREAMER, gst_buffer_new)
GST_FUNC(LIBGSTREAMER, gst_buffer_new_and_alloc)
GST_FUNC(LIBGSTREAMER, gst_buffer_set_qdata)
GST_FUNC(LIBGSTREAMER, gst_bus_timed_pop_filtered)
GST_FUNC(LIBGSTREAMER, gst_caps_append)
GST_FUNC(LIBGSTREAMER, gst_caps_can_intersect)
GST_FUNC(LIBGSTREAMER, gst_caps_from_string)
GST_FUNC(LIBGSTREAMER, gst_caps_get_size)
GST_FUNC(LIBGSTREAMER, gst_caps_get_structure)
GST_FUNC(LIBGSTREAMER, gst_caps_new_any)
GST_FUNC(LIBGSTREAMER, gst_caps_new_empty)
GST_FUNC(LIBGSTREAMER, gst_caps_new_full)
GST_FUNC(LIBGSTREAMER, gst_caps_new_simple)
GST_FUNC(LIBGSTREAMER, gst_caps_unref)
GST_FUNC(LIBGSTREAMER, gst_element_factory_get_klass)
GST_FUNC(LIBGSTREAMER, gst_element_factory_list_filter)
GST_FUNC(LIBGSTREAMER, gst_element_factory_list_get_elements)
GST_FUNC(LIBGSTREAMER, gst_element_factory_make)
GST_FUNC(LIBGSTREAMER, gst_element_get_factory)
GST_FUNC(LIBGSTREAMER, gst_element_get_pad)
GST_FUNC(LIBGSTREAMER, gst_element_get_type)
GST_FUNC(LIBGSTREAMER, gst_element_query_convert)
GST_FUNC(LIBGSTREAMER, gst_element_query_duration)
GST_FUNC(LIBGSTREAMER, gst_element_seek_simple)
GST_FUNC(LIBGSTREAMER, gst_element_set_state)
GST_FUNC(LIBGSTREAMER, gst_event_parse_new_segment)
GST_FUNC(LIBGSTREAMER, gst_flow_get_name)
GST_FUNC(LIBGSTREAMER, gst_init)
GST_FUNC(LIBGSTREAMER, gst_init_check)
GST_FUNC(LIBGSTREAMER, gst_iterator_next)
GST_FUNC(LIBGSTREAMER, gst_message_parse_error)
GST_FUNC(LIBGSTREAMER, gst_mini_object_get_type)
GST_FUNC(LIBGSTREAMER, gst_mini_object_new)
GST_FUNC(LIBGSTREAMER, gst_mini_object_ref)
GST_FUNC(LIBGSTREAMER, gst_mini_object_unref)
GST_FUNC(LIBGSTREAMER, gst_object_get_parent)
GST_FUNC(LIBGSTREAMER, gst_object_unref)
GST_FUNC(LIBGSTREAMER, gst_pad_add_event_probe)
GST_FUNC(LIBGSTREAMER, gst_pad_alloc_buffer)
GST_FUNC(LIBGSTREAMER, gst_pad_get_element_private)
GST_FUNC(LIBGSTREAMER, gst_pad_get_negotiated_caps)
GST_FUNC(LIBGSTREAMER, gst_pad_set_bufferalloc_function)
GST_FUNC(LIBGSTREAMER, gst_pad_set_element_private)
GST_FUNC(LIBGSTREAMER, gst_parse_bin_from_description)
GST_FUNC(LIBGSTREAMER, gst_pipeline_get_bus)
GST_FUNC(LIBGSTREAMER, gst_pipeline_get_type)
GST_FUNC(LIBGSTREAMER, gst_registry_get_default)
GST_FUNC(LIBGSTREAMER, gst_registry_get_feature_list_cookie)
GST_FUNC(LIBGSTREAMER, gst_segment_init)
GST_FUNC(LIBGSTREAMER, gst_segment_set_newsegment)
GST_FUNC(LIBGSTREAMER, gst_segment_to_stream_time)
GST_FUNC(LIBGSTREAMER, gst_structure_copy)
GST_FUNC(LIBGSTREAMER, gst_structure_get_fraction)
GST_FUNC(LIBGSTREAMER, gst_structure_get_int)
GST_FUNC(LIBGSTREAMER, gst_structure_get_value)
GST_FUNC(LIBGSTREAMER, gst_structure_new)
GST_FUNC(LIBGSTREAMER, gst_structure_take_value)
GST_FUNC(LIBGSTREAMER, gst_util_uint64_scale)
GST_FUNC(LIBGSTVIDEO, gst_video_format_get_component_height)
GST_FUNC(LIBGSTVIDEO, gst_video_format_get_component_offset)
GST_FUNC(LIBGSTVIDEO, gst_video_format_get_component_width)
GST_FUNC(LIBGSTVIDEO, gst_video_format_get_row_stride)
GST_FUNC(LIBGSTVIDEO, gst_video_format_parse_caps)

/*
 * Functions that have been defined in the header file. We replace them so that
 * they don't try to use the global gstreamer functions.
 */
#ifdef REPLACE_FUNC
REPLACE_FUNC(gst_buffer_ref);
REPLACE_FUNC(gst_buffer_unref);
REPLACE_FUNC(gst_message_unref);
#endif

#endif // !defined(__APPLE__)
