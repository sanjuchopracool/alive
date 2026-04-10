#ifndef ACTION_IDS_H
#define ACTION_IDS_H

namespace alive::action_ids {

const char k_file_new_action[] = "FILE.NEW";
const char k_file_open_action[] = "FILE.OPEN";
const char k_recent_files_key[] = "FILE.RECENTS";
const char k_clear_recent_files_key[] = "FILE.CLEAR.RECENTS";
const char k_file_close_action[] = "FILE.CLOSE";
const char k_file_save_action[] = "FILE.SAVE";
const char k_file_save_as_action[] = "FILE.SAVE_AS";
const char k_file_import_action[] = "FILE.IMPORT";
const char k_file_export_as_png_action[] = "FILE.EXPORT.PNG";
const char k_file_export_as_webp_action[] = "FILE.EXPORT.WEB_P";

const char k_edit_undo_action[] = "EDIT.UNDO";
const char k_edit_redo_action[] = "EDIT.REDO";
const char k_edit_comp_setting_action[] = "EDIT.COMP_SETTING";

const char k_timeline_play_pause_action[] = "TIMELINE.PLAY_PAUSE";
const char k_timeline_start_work_area_action[] = "TIMELINE.SET_WORK_START";
const char k_timeline_end_work_area_action[] = "TIMELINE.SET_WORK_END";

const char k_timeline_go_next_1_frame_action[] = "TIMELINE.GO_NEXT_1_FRAME";
const char k_timeline_go_next_10_frame_action[] = "TIMELINE.GO_NEXT_10_FRAME";
const char k_timeline_go_back_1_frame_action[] = "TIMELINE.GO_BACK_1_FRAME";
const char k_timeline_go_back_10_frame_action[] = "TIMELINE.GO_BACK_10_FRAME";

const char k_view_zoom_in_action[] = "VIEW.ZOOM.IN";
const char k_view_zoom_out_action[] = "VIEW.ZOOM.OUT";
const char k_view_zoom_fit_action[] = "VIEW.ZOOM.FIT";

} // namespace alive::action_ids
#endif // ACTION_IDS_H
