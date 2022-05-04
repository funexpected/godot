#include "zip_tool.h"

#include "core/os/file_access.h"
#include "core/io/zip_io.h"
#include "core/project_settings.h"

void ZipTool::_bind_methods() {
    ClassDB::bind_method(D_METHOD("list_files", "p_path"), &ZipTool::list_files);
}

Array ZipTool::list_files(const String &p_path) {
    Array result;
    if (!FileAccess::exists(p_path)) {
        return result;
    }
    FileAccess *file;
    zlib_filefunc_def io = zipio_create_io_from_file(&file);
    unzFile zip = unzOpen2(p_path.utf8().get_data(), &io);
    if (!zip) {
        return result;
    }

    unz_global_info64 gi;
	int err = unzGetGlobalInfo64(zip, &gi);
    if (err != UNZ_OK) {
        return result;
    }
	
    for (uint64_t i = 0; i < gi.number_entry; i++) {

		char filename_inzip[256];

		unz_file_info64 file_info;
		err = unzGetCurrentFileInfo64(zip, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
		if (err != UNZ_OK) {
            continue;
        }
		
		String fname = String() + filename_inzip;
        result.append(fname);

        if ((i + 1) < gi.number_entry) {
			unzGoToNextFile(zip);
		}
    }

    return result;
}