#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    string sqlCommand = argv[1];
    string bashCommand = "python3 ~/ultimateALPR-SDK/samples/c++/recognizer/sql.py \\\"" + sqlCommand + "\\\"";
    string fullCommand = "sudo ~/ultimateALPR-SDK/samples/c++/recognizer//writeTerminal -n /dev/pts/1 \"" + bashCommand + "\"";

    unsigned timeOutCount = 0;

    RUN_COMMAND:

    if(system(fullCommand.c_str()) != 0 && timeOutCount <= 10)
    {
        timeOutCount++;
        goto RUN_COMMAND;
    }

    return 0;
}