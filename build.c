

////////////////////////////////////////////////////////////////////////////////
//
// cl build.c && build.exe
//
////////////////////////////////////////////////////////////////////////////////

const char* program_name = "engine_d3d11";

struct Build
{
    const char* name;

    const char* cc_cmd;

    int num_cc_flags;
    const char* cc_flags[256];

    const char* link_cmd;

    int num_link_flags;
    const char* link_flags[256];

    int num_libs;
    const char* libs[256];
};


#define COMMON_COMPILE_FLAGS "/c", "/W4", "/WX", "/EHsc", "/std:c17", "/GS-", "/Gs9999999", "/nologo", "/Isrc"
#define DEBUG_COMPILE_FLAGS "/Od", "/Zi", "/DDEBUG"
#define RELEASE_COMPILE_FLAGS "/O2"
#define COMMON_LINKER_FLAGS "/NODEFAULTLIB", "/STACK:0x100000,0x100000", "/SUBSYSTEM:WINDOWS", "/MACHINE:X64", "/nologo"
#define LIBS "kernel32.lib", "user32.lib", "gdi32.lib", "d3d11.lib", "dxgi.lib", "dxguid.lib"

#define ARRAY_INIT(TYPE, NAME, ...) .num_##NAME = (sizeof( (TYPE[]) { __VA_ARGS__ } ) / sizeof(TYPE)), .NAME = { __VA_ARGS__ }
struct Build build[] =
{
    {
        .name = "clang_debug",

        .cc_cmd = "clang-cl",
        ARRAY_INIT(
            const char*,
            cc_flags,
            COMMON_COMPILE_FLAGS,
            DEBUG_COMPILE_FLAGS,
            "-march=skylake",
        ),

        .link_cmd = "lld-link",
        ARRAY_INIT(
            const char*,
            link_flags,
            COMMON_LINKER_FLAGS,
            "/DEBUG:FULL",
        ),

        ARRAY_INIT(
            const char*,
            libs,
            LIBS,
        ),
    },

    {
        .name = "clang_release",

        .cc_cmd = "clang-cl",
        ARRAY_INIT(
            const char*,
            cc_flags,
            COMMON_COMPILE_FLAGS,
            RELEASE_COMPILE_FLAGS,
            "-march=skylake",
        ),

        .link_cmd = "lld-link",
        ARRAY_INIT(
            const char*,
            link_flags,
            COMMON_LINKER_FLAGS,
        ),
        
        ARRAY_INIT(
            const char*,
            libs,
            LIBS
        ),
    },

    {
        .name = "msvc_debug",

        .cc_cmd = "cl",
        ARRAY_INIT(
            const char*,
            cc_flags,
            COMMON_COMPILE_FLAGS,
            DEBUG_COMPILE_FLAGS,
        ),

        .link_cmd = "link",
        ARRAY_INIT(
            const char*,
            link_flags,
            COMMON_LINKER_FLAGS,
            "/DEBUG:FULL",
        ),
        
        ARRAY_INIT(
            const char*,
            libs,
            LIBS
        ),
    },

    {
        .name = "msvc_release",

        .cc_cmd = "cl",
        ARRAY_INIT(
            const char*,
            cc_flags,
            COMMON_COMPILE_FLAGS,
            RELEASE_COMPILE_FLAGS,
        ),

        .link_cmd = "link",
        ARRAY_INIT(
            const char*,
            link_flags,
            COMMON_LINKER_FLAGS,
        ),
        
        ARRAY_INIT(
            const char*,
            libs,
            LIBS
        ),
    },
    
};




////////////////////////////////////////////////////////////////////////////////
// Impl

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define STRINGIFY(x) STRINGIFY2(x)
#define STRINGIFY2(x) #x
#define LINE_STRING STRINGIFY(__LINE__)
void assert_fn(const int c, const char* msg, ...)
{
    if(!c)
    {
        va_list args;
        va_start(args, msg);
        vfprintf(stderr, msg, args);
        va_end(args);
        ExitProcess(1);
    }
}
#define ASSERT(c, msg, ...) assert_fn((int)(c), (__FILE__ ":" LINE_STRING "  BUILD PROGRAM FAIL: " msg), ##__VA_ARGS__)

#define ARRAY_COUNT(A) (sizeof(A) / sizeof((A)[0]))

void run_cmd(const char* cmd)
{
    STARTUPINFOA startup_info = { .cb = sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION proc_info = {};
    const BOOL ret = CreateProcessA(NULL, (char*)cmd, NULL, NULL, FALSE, 0, NULL, NULL, &startup_info, &proc_info);
    ASSERT(ret, "run_cmd proc failure");
    WaitForSingleObject(proc_info.hProcess, INFINITE);
    CloseHandle(proc_info.hProcess);
    CloseHandle(proc_info.hThread);
}

typedef struct String256Tag
{
    u8 len;
    char s[255];
} String256;

String256 make_str(const char* s)
{
    String256 result;
    const u64 len = strlen(s);
    ASSERT(len < 255, "make_str overflow.");
    result.len = len;
    memcpy(result.s, s, len + 1);
    return result;
}

String256 str_concat_str(const String256 s0, const String256 s1)
{
    String256 result;
    const u64 len = s0.len + s1.len;
    ASSERT(len < 255, "str_concat_str overflow.");
    result.len = len;
    memcpy(result.s, s0.s, s0.len);
    memcpy(result.s + s0.len, s1.s, s1.len);
    result.s[len] = 0;
    return result;
}

String256 str_concat_cstr(const String256 s0, const char* s1)
{
    String256 result;
    const u64 s1_len = strlen(s1);
    const u64 len = s0.len + s1_len;
    ASSERT(len < 255, "str_concat_cstr overflow.");
    result.len = len;
    memcpy(result.s, s0.s, s0.len);
    memcpy(result.s + s0.len, s1, s1_len);
    result.s[len] = 0;
    return result;
}

u64 str_rfind_char(const String256 s, const char c)
{
    u64 result = (u64)-1;
    if(!s.len)
    {
        return result;
    }
    for(u64 i = 0; i < s.len; i++)
    {
        result = s.s[i] == c ? i : result;
    }
    return result;
}

u8 str_begins_with_cstr(const String256 s0, const char* s1)
{
    const u64 s1_len = strlen(s1);
    return strncmp(s0.s, s1, s1_len) == 0;
}

u8 str_ends_with_cstr(const String256 s0, const char* s1)
{
    const u64 s1_len = strlen(s1);
    if(s1_len > s0.len)
    {
        return 0;
    }
    return strncmp(s0.s + s0.len - s1_len, s1, s1_len) == 0;
}

String256 str_slice_left(const String256 s, const u64 pos)
{
    ASSERT(pos <= s.len, "str_slice_left overflow %llu", pos);
    String256 result = s;
    result.len = pos;
    result.s[pos] = 0;
    return result;
}

String256 str_slice_right(const String256 s, const u64 pos)
{
    ASSERT(pos < s.len, "str_slice_right overflow %llu", pos);
    String256 result;
    const u8 len = (u8)(s.len - pos);
    memcpy(result.s, s.s + pos, len);
    result.len = len;
    result.s[len] = 0;
    return result;
}

#define MAKE_STRING256(S) { .len = ARRAY_COUNT(S) - 1, .s = S, }

void find_src_recurse(
    u64* r_num_src,
    String256* r_src,
    const u64 max_r_num_src,
    const String256* dir)
{
    const String256 pattern = str_concat_cstr(*dir, "\\*");

    WIN32_FIND_DATAA find_data = {};
    const HANDLE hfind = FindFirstFileA(pattern.s, &find_data);
    ASSERT(hfind != INVALID_HANDLE_VALUE, "FindFirstFileA failure %i", GetLastError());

    do
    {
        String256 file_path = str_concat_cstr(*dir, "\\");
        file_path = str_concat_cstr(file_path, find_data.cFileName);

        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if(strncmp(find_data.cFileName, ".", 1) != 0 && strncmp(find_data.cFileName, "..", 2) != 0)
            {
                find_src_recurse(
                    r_num_src,
                    r_src,
                    max_r_num_src,
                    &file_path
                );
            }
        }
        else
        {
            ASSERT(*r_num_src < max_r_num_src, "Too many src files.");
            r_src[*r_num_src] = file_path;
            (*r_num_src)++;
        }
    }
    while(FindNextFile(hfind, &find_data) != 0);
    DWORD last_error = GetLastError();
    ASSERT(last_error == ERROR_NO_MORE_FILES, "FindNextFile error: %i", last_error);
    FindClose(hfind);
}

void make_dirs(String256 path)
{
    for(u64 i = 0; i < path.len; i++)
    {
        if(path.s[i] == '\\')
        {
            const char c = path.s[i];
            path.s[i] = 0;
            const BOOL create_directory_success = CreateDirectory(path.s, NULL);
            if(!create_directory_success)
            {
                const u32 error = GetLastError();
                ASSERT(error == ERROR_ALREADY_EXISTS, "'CreateDirectory' failed with error %u", error);
            }
            path.s[i] = c;
        }
    }
}

#define MAX_SRC 1024

int main()
{
    String256 src_dir = MAKE_STRING256("src");

    u64 num_src = 0;
    String256* src = (String256*)calloc(MAX_SRC * sizeof(String256), 1);
    find_src_recurse(
        &num_src,
        src,
        MAX_SRC,
        &src_dir
    );

    u64 num_c_files = 0;
    String256* c_files = (String256*)calloc(MAX_SRC * sizeof(String256), 1);

    u64 num_hlsl_files = 0;
    String256* hlsl_files = (String256*)calloc(MAX_SRC * sizeof(String256), 1);

    u64 num_obj_files = 0;
    String256* obj_files = (String256*)calloc(MAX_SRC * sizeof(String256), 1);

    // Get files of each type.
    for(u64 i = 0; i < num_src; i++)
    {
        const String256 src_path = src[i];
        ASSERT(str_begins_with_cstr(src_path, "src\\"), "Expected 'src\\' in beginning of src file '%s'", src[i].s);

        if(str_ends_with_cstr(src_path, ".c"))
        {
            ASSERT(num_c_files <= MAX_SRC, "c_files overflow.");
            c_files[num_c_files] = src_path;
            num_c_files++;
        }

        if(str_ends_with_cstr(src_path, ".hlsl"))
        {
            ASSERT(num_hlsl_files <= MAX_SRC, "hlsl_files overflow.");
            hlsl_files[num_hlsl_files] = src_path;
            num_hlsl_files++;
        }
    }
    
    // Compile shaders.
    for(u64 i = 0; i < num_hlsl_files; i++)
    {
        const String256 hlsl_file = hlsl_files[i];
        String256 name = str_slice_left(hlsl_file, str_rfind_char(hlsl_file, '.'));
        name = str_slice_right(name, str_rfind_char(name, '\\') + 1);

        char cmd[4096] = {};
        s32 num_written = snprintf(
            cmd,
            sizeof(cmd),
            "fxc.exe /nologo /T vs_5_0 /E vs /O3 /WX /Zpc /Ges /Fh src\\platform_win32\\shaders\\generated\\d3d11_vshader_%s.h /Vn d3d11_vshader_%s /Qstrip_reflect /Qstrip_debug /Qstrip_priv %s",
            name.s,
            name.s,
            hlsl_file.s);
        ASSERT(num_written >= 0 && num_written >= 0 && num_written < sizeof(cmd), "cmd overflow.");
        run_cmd(cmd);

        num_written = snprintf(
            cmd,
            sizeof(cmd),
            "fxc.exe /nologo /T ps_5_0 /E ps /O3 /WX /Zpc /Ges /Fh src\\platform_win32\\shaders\\generated\\d3d11_pshader_%s.h /Vn d3d11_pshader_%s /Qstrip_reflect /Qstrip_debug /Qstrip_priv %s",
            name.s,
            name.s,
            hlsl_file.s);
        ASSERT(num_written >= 0 && num_written >= 0 && num_written < sizeof(cmd), "cmd overflow.");
        run_cmd(cmd);
    }

    u32 compile_error = 0;

    // Build C files for each build config.
    for(u64 i_build = 0; i_build < ARRAY_COUNT(build); i_build++)
    {
        const struct Build* cur_build = &build[i_build];

        num_obj_files = 0;

        String256 inter_dir = MAKE_STRING256("build\\intermediate_");
        inter_dir = str_concat_cstr(inter_dir, cur_build->name);
        inter_dir = str_concat_cstr(inter_dir, "\\");

        String256 deploy_dir = MAKE_STRING256("build\\deploy_");
        deploy_dir = str_concat_cstr(deploy_dir, cur_build->name);
        deploy_dir = str_concat_cstr(deploy_dir, "\\");

        u64 num_compiles = 0;
        HANDLE compile_procs[MAX_SRC];
        HANDLE compile_threads[MAX_SRC];
        HANDLE compile_stdouts_read[MAX_SRC];
        HANDLE compile_stdouts_write[MAX_SRC];
        
        for(u64 i = 0; i < num_c_files; i++)
        {
            const String256 c_file = c_files[i];

            String256 obj_path = str_concat_str(inter_dir, c_file);
            obj_path = str_concat_cstr(str_slice_left(obj_path, str_rfind_char(obj_path, '.')), ".obj");

            ASSERT(num_obj_files < MAX_SRC, "obj_files overflow.");
            obj_files[num_obj_files] = obj_path;
            num_obj_files++;

            String256 pdb_path = str_concat_str(inter_dir, c_file);
            pdb_path = str_concat_cstr(str_slice_left(pdb_path, str_rfind_char(pdb_path, '.')), ".pdb");

            make_dirs(obj_path);

            char cmd[4096] = {};
            char* cmd_cur = cmd;
            char* cmd_end = cmd + sizeof(cmd) - 1;

            {
                const s32 num_written = snprintf(cmd_cur, cmd_end - cmd_cur, "%s ", cur_build->cc_cmd);
                ASSERT(num_written >= 0 && num_written < cmd_end - cmd_cur, "cmd overflow.");
                cmd_cur += num_written;
            }

            for(u64 i_flag = 0; i_flag < (u64)cur_build->num_cc_flags; i_flag++)
            {
                const s32 num_written = snprintf(cmd_cur, cmd_end - cmd_cur, "%s ", cur_build->cc_flags[i_flag]);
                ASSERT(num_written >= 0 && num_written < cmd_end - cmd_cur, "cmd overflow.");
                cmd_cur += num_written;
            }

            {
                const s32 num_written = snprintf(cmd_cur, cmd_end - cmd_cur, "/Fo%s /Fd%s %s",
                                                 obj_path.s,
                                                 pdb_path.s,
                                                 c_file.s);
                ASSERT(num_written >= 0 && num_written < cmd_end - cmd_cur, "cmd overflow.");
                cmd_cur += num_written;
            }

            printf("Compiling '%s'\n", c_file.s);
            fflush(stdout);

            // Create compiler process.
            {
                // Create stdout pipe for child.
                SECURITY_ATTRIBUTES sa =
                {
                    .nLength = sizeof(SECURITY_ATTRIBUTES),
                    .bInheritHandle = TRUE,
                };

                BOOL success = CreatePipe(
                    &compile_stdouts_read[num_compiles],
                    &compile_stdouts_write[num_compiles],
                    &sa,
                    0);
                ASSERT(success, "'CreatePipe' failed.");
                success = SetHandleInformation(
                    compile_stdouts_read[num_compiles],
                    HANDLE_FLAG_INHERIT,
                    0);
                ASSERT(success, "'SetHandleInformation' failed.");

                STARTUPINFOA startup_info = {
                    .cb = sizeof(STARTUPINFOA),
                    .hStdError = compile_stdouts_write[num_compiles],
                    .hStdOutput = compile_stdouts_write[num_compiles],
                    .dwFlags = STARTF_USESTDHANDLES,
                };
                PROCESS_INFORMATION proc_info = {};
                const BOOL ret = CreateProcessA(NULL, (char*)cmd, NULL, NULL, TRUE, 0, NULL, NULL, &startup_info, &proc_info);
                ASSERT(ret, "run_cmd proc failure");

                // This process must close the stdout write handle (we don't need it).
                // Otherwise, we cannot wait for the child to end.
                CloseHandle(compile_stdouts_write[num_compiles]);

                compile_procs[num_compiles] = proc_info.hProcess;
                compile_threads[num_compiles] = proc_info.hThread;
                num_compiles++;
            }
        }

        WaitForMultipleObjects(num_compiles, compile_procs, TRUE, INFINITE);

        String256 program_name_exe = str_concat_cstr(make_str(program_name), ".exe");

        const HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
        for(u64 i = 0; i < num_compiles; i++)
        {
            {
                u32 exit_code;
                u32 success = GetExitCodeProcess(compile_procs[i], &exit_code);
                ASSERT(success, "'GetExitCodeProcess' failed with error %u.", GetLastError());

                compile_error = compile_error || (exit_code != 0);
            }

            u8 buf[4096];
            while(1)
            {
                u32 num_read;
                u32 success = ReadFile(
                    compile_stdouts_read[i],
                    buf,
                    sizeof(buf) - 1,
                    &num_read,
                    NULL);
                u32 err = GetLastError();
                ASSERT(success || err == ERROR_BROKEN_PIPE, "'ReadFile' failed with error %u.", err);
                if(num_read == 0)
                {
                    break;
                }

                buf[num_read] = 0;

                printf("%s", buf);
            }

            CloseHandle(compile_procs[i]);
            CloseHandle(compile_threads[i]);
            CloseHandle(compile_stdouts_read[i]);
        }

        if(compile_error)
        {
            break;
        }

        {
            char cmd[4096] = {};
            char* cmd_cur = cmd;
            char* cmd_end = cmd + sizeof(cmd) - 1;

            {
                const s32 num_written = snprintf(cmd_cur, cmd_end - cmd_cur, "%s ", cur_build->link_cmd);
                ASSERT(num_written >= 0 && num_written < cmd_end - cmd_cur, "cmd overflow.");
                cmd_cur += num_written;
            }

            for(u64 i = 0; i < (u64)cur_build->num_link_flags; i++)
            {
                const s32 num_written = snprintf(cmd_cur, cmd_end - cmd_cur, "%s ", cur_build->link_flags[i]);
                ASSERT(num_written >= 0 && num_written < cmd_end - cmd_cur, "cmd overflow.");
                cmd_cur += num_written;
            }

            for(u64 i = 0; i < (u64)cur_build->num_libs; i++)
            {
                const s32 num_written = snprintf(cmd_cur, cmd_end - cmd_cur, "%s ", cur_build->libs[i]);
                ASSERT(num_written >= 0 && num_written < cmd_end - cmd_cur, "cmd overflow.");
                cmd_cur += num_written;
            }

            {
                const s32 num_written = snprintf(cmd_cur, cmd_end - cmd_cur, "/OUT:%s%s ", inter_dir.s, program_name_exe.s);
                ASSERT(num_written >= 0 && num_written < cmd_end - cmd_cur, "cmd overflow.");
                cmd_cur += num_written;
            }

            for(u64 i = 0; i < num_obj_files; i++)
            {
                const s32 num_written = snprintf(cmd_cur, cmd_end - cmd_cur, "%s ", obj_files[i].s);
                ASSERT(num_written >= 0 && num_written < cmd_end - cmd_cur, "cmd overflow.");
                cmd_cur += num_written;
            }

            printf("Linking...\n");
            run_cmd(cmd);
        }

        {
            String256 src = str_concat_str(inter_dir, program_name_exe);
            String256 dst = str_concat_str(deploy_dir, program_name_exe);

            make_dirs(deploy_dir);

            const BOOL success = CopyFile(
                src.s,
                dst.s,
                FALSE);
            ASSERT(success, "'CopyFile' failed with error %u.", GetLastError());
        }
    }
}
