#include <cstdio>
#include <cstdlib>
#include <string>

std::string createCommandFZF() {
    std::string projectsPath = "~/projects";
    std::string dotfilePath = "~/dotfiles";
    std::string excludes[3] = {"'*/.git*'", "'*/.godot*'", "'*/.vscode*'"};
    std::string command = " ( tmux ls 2>/dev/null; find " + projectsPath + " -maxdepth 1 ";
    int excludesSize = sizeof(excludes) / sizeof(excludes[0]);

    for (int i = 0; i < excludesSize; i++) {
        if (i != excludesSize - 1) {
            command += " -path " + excludes[i] + " -o ";
        } else {
            command += " -path " + excludes[i] + " -prune -o ";
        }
    }

    command += " -print -type d; find " + dotfilePath + " -maxdepth 0 -print -type d;) | fzf";
    return command;
}

void cleanProjectString(std::string &projectPath) {
    if (!projectPath.empty() && projectPath.back() == '\n') {
        projectPath.pop_back();
    }

    size_t strPos = projectPath.find(':');
    if (strPos != std::string::npos) {
        projectPath.erase(strPos);
    }
}

void handleInsideTmuxSession(const std::string &projectPath, bool doesTmuxSessionExist) {
    if (!doesTmuxSessionExist) {
        std::string createCommand = "tmux new-session -ds " + projectPath + " -c " + projectPath;
        system(createCommand.c_str());
    }
    std::string attachCommand = "tmux switch-client -t " + projectPath;
    system(attachCommand.c_str());
}

void handleOutsideTmuxSession(const std::string &projectPath, bool doesTmuxSessionExist) {
    if (!doesTmuxSessionExist) {
        std::string createCommand = "tmux new-session -ds " + projectPath + " -c " + projectPath;
        system(createCommand.c_str());
    }
    std::string attachCommand = "tmux attach -t " + projectPath;
    system(attachCommand.c_str());
}

int main() {
    std::string command = createCommandFZF();

    FILE *pipe = popen(command.c_str(), "r");

    char buffer[128];
    std::string projectPath = "";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        projectPath += buffer;
    }

    int status = pclose(pipe);

    if (projectPath.empty())
        return 0;

    cleanProjectString(projectPath);

    std::string tmuxExistCommand = "tmux has-session -t " + projectPath + " 2>/dev/null";
    // Zero means that the command was able to execute and it is a session
    bool doesTmuxSessionExist = system(tmuxExistCommand.c_str()) == 0 ? true : false;

    if (std::getenv("TMUX") != nullptr) {
        handleInsideTmuxSession(projectPath, doesTmuxSessionExist);
    } else {
        handleOutsideTmuxSession(projectPath, doesTmuxSessionExist);
    }
    return 0;
}
