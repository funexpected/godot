{
    // See https://go.microsoft.com/fwlink/?LinkId=733558
    // for the documentation about the tasks.json format
    "version": "2.0.0",
    "tasks": [
        {
            "label": "build osx editor",
            "type": "shell",
            "command": "scons",
            "args": ["platform=windows"],
            "group": {
                "kind": "build",
                "isDefault": true
            }
        }
    ]
}