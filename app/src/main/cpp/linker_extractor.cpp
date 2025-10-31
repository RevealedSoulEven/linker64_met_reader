#include <jni.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <link.h>
#include <dlfcn.h>
#include <unistd.h>
#include <cinttypes>
#include <cstdlib>

std::string read_file_raw(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return "[UNABLE TO READ: " + path + "]";
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::vector<std::string> dl_libs;
int phdr_callback_raw(struct dl_phdr_info *info, size_t size, void *data) {
    char entry[512];
    snprintf(entry, sizeof(entry), "%-50s Base: 0x%016" PRIxPTR,
             info->dlpi_name ? info->dlpi_name : "[main]",
             (uintptr_t)info->dlpi_addr);
    dl_libs.push_back(entry);
    return 0;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_linkerextractor_MainActivity_collectAllLinkerData(JNIEnv *env, jobject thiz) {
    std::string result;
    
    // /proc/self/maps
    result += "=== /proc/self/maps ===\n";
    result += read_file_raw("/proc/self/maps") + "\n";
    
    // dl_iterate_phdr
    result += "=== DL_ITERATE_PHDR ===\n";
    dl_libs.clear();
    dl_iterate_phdr(phdr_callback_raw, nullptr);
    for (const auto& entry : dl_libs) {
        result += entry + "\n";
    }
    result += "\n";
    
    // Environment
    result += "=== ENVIRONMENT ===\n";
    extern char **environ;
    for (char **env = environ; *env; env++) {
        result += std::string(*env) + "\n";
    }
    result += "\n";
    
    // File access test
    result += "=== FILE ACCESS ===\n";
    const char* test_files[] = {
        "/system/bin/linker64", "/system/bin/linker", 
        "/data/adb/magisk", "libmemtrack.so", nullptr
    };
    for (int i = 0; test_files[i]; i++) {
        if (access(test_files[i], F_OK) == 0) {
            result += "EXISTS: " + std::string(test_files[i]) + "\n";
        }
    }
    
    return env->NewStringUTF(result.c_str());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_linkerextractor_MainActivity_exportDataToFile(JNIEnv *env, jobject thiz, jstring j_path) {
    const char* path = env->GetStringUTFChars(j_path, 0);
    jstring all_data = Java_com_example_linkerextractor_MainActivity_collectAllLinkerData(env, thiz);
    const char* data = env->GetStringUTFChars(all_data, 0);
    
    FILE* file = fopen(path, "w");
    if (file) {
        fputs(data, file);
        fclose(file);
        env->ReleaseStringUTFChars(all_data, data);
        env->ReleaseStringUTFChars(j_path, path);
        return env->NewStringUTF("SUCCESS: Data exported to file");
    } else {
        env->ReleaseStringUTFChars(all_data, data);
        env->ReleaseStringUTFChars(j_path, path);
        return env->NewStringUTF("ERROR: Could not write to file");
    }
}