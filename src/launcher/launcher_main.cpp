#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <cstdlib>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "shell32.lib")
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#endif

// Find a free TCP port on localhost
int find_free_port(int default_port = 8080) {
#if defined(_WIN32)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return default_port;
    }
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        WSACleanup();
        return default_port;
    }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = 0; // OS binds to ephemeral free port

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(sock);
        WSACleanup();
        return default_port;
    }

    int addr_len = sizeof(addr);
    if (getsockname(sock, (sockaddr*)&addr, &addr_len) == SOCKET_ERROR) {
        closesocket(sock);
        WSACleanup();
        return default_port;
    }
    int port = ntohs(addr.sin_port);
    closesocket(sock);
    WSACleanup();
    return port;
#else
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return default_port;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = 0;

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        return default_port;
    }

    socklen_t len = sizeof(addr);
    if (getsockname(sock, (struct sockaddr*)&addr, &len) < 0) {
        close(sock);
        return default_port;
    }
    int port = ntohs(addr.sin_port);
    close(sock);
    return port;
#endif
}

#if defined(_WIN32)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hInstance; (void)hPrevInstance; (void)lpCmdLine; (void)nCmdShow;

    int port = find_free_port(8080);
    std::wstring port_w = std::to_wstring(port);

    // Command line to launch workbench_server.exe
    std::wstring cmd = L"workbench_server.exe --port " + port_w;

    STARTUPINFOW si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE; // Hidden window: no terminal, no powershell!

    BOOL created = CreateProcessW(
        NULL,
        &cmd[0],
        NULL,
        NULL,
        FALSE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &pi
    );

    if (!created) {
        MessageBoxW(NULL, L"Failed to start workbench_server.exe. Ensure all files in dist/ are present.", L"Unified Engineering Workbench Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    // Give server ~300ms to bind port
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // Open default browser to the localhost URL
    std::wstring url = L"http://127.0.0.1:" + port_w + L"/";
    ShellExecuteW(NULL, L"open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);

    // Wait until server process exits
    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}
#else
// Linux / POSIX entry point
pid_t g_server_pid = -1;

void handle_signal(int sig) {
    (void)sig;
    if (g_server_pid > 0) {
        kill(g_server_pid, SIGTERM);
    }
    exit(0);
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    int port = find_free_port(8080);

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    pid_t pid = fork();
    if (pid == 0) {
        // Child: run server
        std::string port_str = std::to_string(port);
        char* args[] = {
            const_cast<char*>("workbench_server"),
            const_cast<char*>("--port"),
            const_cast<char*>(port_str.c_str()),
            nullptr
        };
        execv("./workbench_server", args);
        // Fallback search in ./build
        execv("./build/workbench_server", args);
        _exit(1);
    } else if (pid > 0) {
        g_server_pid = pid;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        std::string url = "http://127.0.0.1:" + std::to_string(port) + "/";
        std::cout << "[Launcher] Unified Engineering Workbench started at " << url << std::endl;

        // Try opening default browser
        std::string open_cmd = "xdg-open " + url + " > /dev/null 2>&1 &";
        int sys_ret = system(open_cmd.c_str());
        (void)sys_ret;

        // Wait for server child process
        int status = 0;
        waitpid(pid, &status, 0);
    } else {
        std::cerr << "[Launcher] Failed to fork server process." << std::endl;
        return 1;
    }
    return 0;
}
#endif
