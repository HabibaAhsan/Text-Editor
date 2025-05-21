#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <windows.h>
#include <fstream>

using namespace std;
#ifdef _WIN32
#include <conio.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <termios.h>
#include <unistd.h>
#endif

class Node {
public:
    char data;
    Node* next;
    Node* prev;

    Node(char ch) : data(ch), next(nullptr), prev(nullptr) {}
};

class LinkedList {
private:
    Node* head;
    Node* tail;

public:
    class Iterator {
    private:
        Node* current;
    public:
        Iterator(Node* node) : current(node) {}
        char& operator*() 
        { 
            return current->data; 
        }
        Iterator& operator++() 
        {
            if (current) current = current->next;
            return *this;
        }
        Iterator& operator--() 
        {
            if (current) current = current->prev;
            return *this;
        }
        bool operator!=(const Iterator& other) const 
        {
            return current != other.current;
        }
        bool operator==(const Iterator& other) const 
        {
            return current == other.current;
        }
        Node* getNode() const {
            return current;
        }
    };
    LinkedList() : head(nullptr), tail(nullptr) {}

    void insertChar(Iterator& iter, char ch) {
        Node* newNode = new Node(ch);
        Node* current = iter.getNode();

        if (!head) {
            head = tail = newNode;
        }
        else if (!current) {
            newNode->next = head;
            head->prev = newNode;
            head = newNode;
        }
        else {
            newNode->next = current->next;
            newNode->prev = current;
            if (current->next) current->next->prev = newNode;
            current->next = newNode;
            if (!newNode->next) tail = newNode;
        }
        iter = Iterator(newNode);
    }
    void deleteChar(Iterator& iter) {
        Node* current = iter.getNode();

        if (!current) return;

        if (current == head) {
            head = head->next;
            if (head) head->prev = nullptr;
            delete current;
            iter = Iterator(head);
        }
        else {
            Node* prevNode = current->prev;
            Node* nextNode = current->next;

            if (prevNode) prevNode->next = nextNode;
            if (nextNode) nextNode->prev = prevNode;

            if (current == tail) tail = prevNode;

            delete current;
            iter = Iterator(prevNode);
        }
    }
    int distance(const Iterator& start, const Iterator& end) const {
        int dist = 0;
        Node* current = start.getNode();
        while (current != end.getNode()) {
            dist++;
            current = current->next;
            if (current == nullptr) {
                break;
            }
        }
        return dist;
    }

    Iterator begin() {
        return Iterator(head);
    }
    Iterator end() {
        return Iterator(nullptr);
    }
    Iterator last() {
        return Iterator(tail);
    }
    bool printLine(const Iterator& cursor, bool cursorPrinted) const {
        Node* temp = head;

        while (temp) {
            cout << temp->data;
            if (cursor.getNode() == temp && !cursorPrinted) {
                cout << "|";
                cursorPrinted = true;
            }
            temp = temp->next;
        }
        return cursorPrinted;
    }
    void deleteLine() {
        Node* temp = head;
        while (temp) {
            Node* next = temp->next;
            delete temp;
            temp = next;
        }
        head = tail = nullptr;
    }
    bool isEmpty() const {
        return head == nullptr;
    }

    string getLineContent() const {
        string content;
        Node* temp = head;
        while (temp) {
            content += temp->data;
            temp = temp->next;
        }
        return content;
    }
};


class FileManager {
private:
    string currentFileName;
    bool modified;

public:
    FileManager() : modified(false) {}

    bool loadFile(const std::string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        currentFileName = filename;
        modified = false;
        return true;
    }

    bool saveFile(const std::string& filename, const std::vector<LinkedList>& lines) {
        ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        for (const auto& line : lines) {
            file << line.getLineContent() << '\n';
        }
        currentFileName = filename;
        modified = false;
        return true;
    }

    bool hasUnsavedChanges() const {
        return modified;
    }

    void markAsModified() {
        modified = true;
    }

    string getCurrentFileName() const {
        return currentFileName.empty() ? "[No File]" : currentFileName;
    }
};

class SearchEngine {
private:
    string lastPattern;
    size_t lastMatchLine;
    size_t lastMatchColumn;

public:
    SearchEngine() : lastPattern(""), lastMatchLine(0), lastMatchColumn(0) {}

    bool search(const string& pattern, vector<LinkedList>& lines, int& currentLine, LinkedList::Iterator& charCursor,int cursorPos) 
    {
        lastPattern = pattern;
        string content = lines[currentLine].getLineContent();

        size_t pos = content.find(pattern, cursorPos);
        if (pos != string::npos) {
            charCursor = lines[currentLine].begin();
            for (size_t j = 0; j < pos; ++j) {
                ++charCursor;
            }
            lastMatchLine = currentLine;
            lastMatchColumn = pos;
            return true;
        }

        for (int i = currentLine + 1; i < lines.size(); ++i) {
            content = lines[i].getLineContent();
            pos = content.find(pattern);
            if (pos != string::npos) {
                currentLine = i;
                charCursor = lines[i].begin();
                for (size_t j = 0; j < pos; ++j) {
                    ++charCursor;
                }
                lastMatchLine = i;
                lastMatchColumn = pos;
                return true;
            }
        }
        return false;
    }


    bool findNext(vector<LinkedList>& lines, int& currentLine, LinkedList::Iterator& charCursor,int cursorPos) 
    {
        if (lastPattern.empty()) return false;
        return search(lastPattern, lines, currentLine, charCursor, cursorPos);
    }

    bool findPrevious(vector<LinkedList>& lines, int& currentLine, LinkedList::Iterator& charCursor,int cursorPos) 
    {
        if (lastPattern.empty()) return false;

        string content = lines[currentLine].getLineContent();

        size_t pos = content.rfind(lastPattern, cursorPos - 2);
        if (pos != string::npos) {
            charCursor = lines[currentLine].begin();
            for (size_t j = 0; j < pos; ++j) {
                ++charCursor;
            }
            lastMatchLine = currentLine;
            lastMatchColumn = pos;
            return true;
        }
        return false;
    }

    bool replace(const string& old, const string& newStr, LinkedList& line, bool global = false) 
    {
        string content = line.getLineContent();
        size_t pos = content.find(old);
        if (pos == string::npos) return false;

        while (pos != string::npos) {
            line.deleteLine();
            LinkedList::Iterator iter = line.end();
            for (size_t i = 0; i < pos; ++i) {
                line.insertChar(iter, content[i]);
            }
            for (char ch : newStr) {
                line.insertChar(iter, ch);
            }
            for (size_t i = pos + old.size(); i < content.size(); ++i) {
                line.insertChar(iter, content[i]);
            }

            if (!global) break;
            content = line.getLineContent();
            pos = content.find(old, pos + newStr.size());
        }
        return true;
    }
};

struct EditorStatus {
    enum Mode { INSERT, NORMAL };
    Mode currentMode;
    size_t cursorLine;
    size_t cursorColumn;
    size_t totalLines;
    string lastCommand;
};


class TextEditor {
private:
    vector<LinkedList> lines;
    int currentLine;
    LinkedList::Iterator charCursor;
    bool insertMode;
    string copyBuffer;
    EditorStatus status;
    FileManager fileManager;
    SearchEngine searchEngine;

    void updateModifiedStatus() 
    {
        fileManager.markAsModified();
    }

    bool isWordCharacter(char c) 
    {
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
            return true;
        }
        return false;
    }

    bool isPunctuation(char c) 
    {
        if (c == '.' || c == ',' || c == ';' || c == ':' || c == '!' || c == '?' ||
            c == '"' || c == '\'' || c == '(' || c == ')' || c == '-' || c == '_') {
            return true;
        }
        return false;
    }

public:
    TextEditor() : currentLine(0), insertMode(true), charCursor(nullptr), copyBuffer("") 
    {
        lines.emplace_back();
        charCursor = lines[0].begin();
        status = { EditorStatus::INSERT, 0, 0, 1, "" };
    }

    // Search commands
    bool search(const string& pattern) 
    {
        int cursorPos = status.cursorColumn;
        return searchEngine.search(pattern, lines, currentLine, charCursor, cursorPos);
    }
    bool findNext() 
    {
        int cursorPos = status.cursorColumn;
        return searchEngine.findNext(lines, currentLine, charCursor, cursorPos);
    }
    bool findPrevious() 
    {
        int cursorPos = status.cursorColumn;
        return searchEngine.findPrevious(lines, currentLine, charCursor, cursorPos);
    }

    // Replace commands
    void replace(const string& old, const string& newStr, bool global = false) {
        searchEngine.replace(old, newStr, lines[currentLine], global);
    }

    // Advanced commands
    void joinLines() {
        if (currentLine < lines.size() - 1) {
            LinkedList::Iterator iter = lines[currentLine + 1].begin();
            while (iter != lines[currentLine + 1].end()) {
                LinkedList::Iterator it = lines[currentLine].end();

                lines[currentLine].insertChar(it, *iter);
                ++iter;
            }
            lines.erase(lines.begin() + currentLine + 1);
        }
    }

    void indentLine(bool increase) {
        char indentChar = '\t';
        if (increase) {
            LinkedList::Iterator iter = lines[currentLine].begin();
            --iter;
            lines[currentLine].insertChar(iter, indentChar);
        }
        else {
            if (*lines[currentLine].begin() == indentChar) {
                LinkedList::Iterator iter = lines[currentLine].begin();

                lines[currentLine].deleteChar(iter);
            }
        }
    }

    void deleteLineNumber(size_t lineNum) {
        lineNum--;
        if (lineNum < lines.size()) {
            lines.erase(lines.begin() + lineNum);
            if (currentLine >= lines.size()) {
                currentLine = lines.size() - 1;
            }
            charCursor = lines[currentLine].begin();
        }
    }

    // file commands
    bool handleFileCommand(const string& cmd) {
        
        if (cmd.rfind("w ", 0) == 0) { 
            string filename = cmd.substr(2);
            if (fileManager.saveFile(filename, lines)) {
                cout << "file : " << filename << " saved";
                return true;
            }

        }
        else if (cmd == "q") {
            if (fileManager.hasUnsavedChanges()) {
                cout << "Warning: Unsaved changes -- Use :q! to force quit\n";
                Sleep(1000);
            }
            else {
                exit(0);
            }
        }
        else if (cmd == "q!") { 
            exit(0);
        }
        else if (cmd == "wq") { 
            if (!fileManager.getCurrentFileName().empty() &&
                fileManager.saveFile(fileManager.getCurrentFileName(), lines)) {
                exit(0);
            }

        }
        else if (cmd.rfind("e ", 0) == 0) { 
            string filename = cmd.substr(2);
            if (fileManager.loadFile(filename)) {
                lines.clear();
                ifstream file(filename);
                string line;
                while (std::getline(file, line)) {
                    LinkedList linkedLine;
                    LinkedList::Iterator iter = linkedLine.end();

                    for (char ch : line) {
                        linkedLine.insertChar(iter, ch);
                    }
                    lines.push_back(linkedLine);
                }
                currentLine = 0;
                charCursor = lines[currentLine].begin();
                return true;
            }

        }
        return false;
    }

    //insert
    void insertChar(char ch) 
    {
        lines[currentLine].insertChar(charCursor, ch);
        updateModifiedStatus();
    }

    //delete
    void deleteChar() {
        if (charCursor == nullptr && currentLine > 0) 
        {
            LinkedList::Iterator previousLineEnd = lines[currentLine - 1].last();
            LinkedList::Iterator currentLineCursor = lines[currentLine].begin();
            while (currentLineCursor != lines[currentLine].end()) {
                lines[currentLine - 1].insertChar(previousLineEnd, *currentLineCursor);
                ++currentLineCursor;
            }
            lines.erase(lines.begin() + currentLine);
            currentLine--;
            charCursor = lines[currentLine].last();
        }
        else {
            lines[currentLine].deleteChar(charCursor);
        }
        updateModifiedStatus();
    }

    void deleteCurrentLine() {
        if (lines.size() > 1) {
            lines.erase(lines.begin() + currentLine);
            if (currentLine >= lines.size()) {
                currentLine = lines.size() - 1;
            }
            charCursor = lines[currentLine].begin();
        }
        else {
            lines[currentLine] = LinkedList();
            charCursor = lines[currentLine].begin();
        }
        updateModifiedStatus();
    }
    void deleteFromCursorToEnd() {
        LinkedList::Iterator endIter = lines[currentLine].end();

        while (charCursor != endIter) {
            LinkedList::Iterator nextChar = charCursor;
            ++nextChar;
            lines[currentLine].deleteChar(charCursor);
            charCursor = nextChar;
        }
        updateModifiedStatus();
    }

    // movement
    void moveUp() {
        if (currentLine > 0) {
            currentLine--;
            charCursor = lines[currentLine].begin();
        }
    }
    void moveDown() {
        if (currentLine < lines.size() - 1) {
            currentLine++;
            charCursor = lines[currentLine].begin();
        }
    }
    void moveLeft() {
        if (charCursor != lines[currentLine].begin()) 
            --charCursor;
        if (charCursor == lines[currentLine].begin())
            charCursor = nullptr;
    }
    void moveRight() {
        if (charCursor == nullptr && lines[currentLine].begin() != nullptr) 
        {
            charCursor = lines[currentLine].begin();
        }
        else if (charCursor != lines[currentLine].end() && charCursor.getNode()->next != nullptr) 
        {
            ++charCursor;
        }
    }

    void moveToStartOfLine() {
        charCursor = lines[currentLine].begin();
        status.cursorColumn = 0;
    }

    void moveToEndOfLine() {
        if (!lines[currentLine].isEmpty()) {
            charCursor = lines[currentLine].last();
        }
        status.cursorColumn = lines[currentLine].getLineContent().size();
    }


    void moveToNextWord() {
        while (charCursor != lines[currentLine].end() && !isWordCharacter(*charCursor)) {
            ++charCursor;
        }
        while (charCursor != lines[currentLine].end() && isWordCharacter(*charCursor)) {
            ++charCursor;
        }
        while (charCursor != lines[currentLine].end() && !isWordCharacter(*charCursor)) {
            ++charCursor;
        }
        if (charCursor == lines[currentLine].end() && currentLine < lines.size() - 1) {
            currentLine++;
            charCursor = lines[currentLine].begin();
        }
    }

    void moveToPreviousWord() {
        while (charCursor != lines[currentLine].begin() && !isWordCharacter(*charCursor)) {
            --charCursor;
        }
        while (charCursor != lines[currentLine].begin() && isWordCharacter(*charCursor)) {
            --charCursor;
        }
        if (charCursor != lines[currentLine].begin() && isWordCharacter(*charCursor)) {
            while (charCursor != lines[currentLine].begin() && isWordCharacter(*charCursor)) {
                --charCursor;
            }
        }
        if (charCursor == lines[currentLine].begin()) {
            --charCursor;
        }
    }

    void moveToWordEnd() {
        while (charCursor != lines[currentLine].end() && isWordCharacter(*charCursor)) {
            ++charCursor;
        }
        if (charCursor == lines[currentLine].end()) {
            if (currentLine < lines.size() - 1) {
                currentLine++;
                charCursor = lines[currentLine].begin();
            }
        }
        else {

            if (charCursor != lines[currentLine].begin()) {
                --charCursor;
            }
        }
        status.cursorColumn = lines[currentLine].distance(lines[currentLine].begin(), charCursor) + 1;
    }

    //new line
    void newLine() {
        lines.insert(lines.begin() + currentLine + 1, LinkedList());
        currentLine++;
        charCursor = lines[currentLine].begin();
    }

    // modes
    void enterInsertMode() {
        status.currentMode = EditorStatus::INSERT;
        insertMode = true;
    }
    void exitInsertMode() {
        status.currentMode = EditorStatus::NORMAL;
        insertMode = false;
    }
    bool isInsertMode() const {
        return insertMode;
    }

    // copy paste
    void yankLine() {
        if (!lines[currentLine].getLineContent().empty()) {
            copyBuffer = lines[currentLine].getLineContent();
            status.lastCommand = "yy";
        }
    }

    void pasteAfter() {
        if (!copyBuffer.empty()) {
            lines.insert(lines.begin() + currentLine + 1, LinkedList());
            LinkedList::Iterator iter = lines[currentLine + 1].begin();

            for (char ch : copyBuffer) {
                lines[currentLine + 1].insertChar(iter, ch);
            }
            status.lastCommand = "p";
            status.totalLines++;
        }
        updateModifiedStatus();
    }

    void pasteBefore() {
        if (!copyBuffer.empty()) {
            lines.insert(lines.begin() + currentLine, LinkedList());
            LinkedList::Iterator iter = lines[currentLine].begin();

            for (char ch : copyBuffer) {
                lines[currentLine].insertChar(iter, ch);
            }
            status.lastCommand = "P";
            status.totalLines++;
            currentLine++;
        }
        updateModifiedStatus();
    }

    // status
    void updateStatusLine() {

        status.cursorLine = currentLine + 1;

        int columnPosition = lines[currentLine].distance(lines[currentLine].begin(), charCursor) + 1;
        if (charCursor == nullptr)
            columnPosition = 0;
        status.cursorColumn = columnPosition;
        status.totalLines = lines.size();
    }

    string getStatusLineText() const {
        string modeText;
        if (insertMode) {
            modeText = "INSERT MODE";
        }
        else {
            modeText = "NORMAL MODE";
        }
        string fileName = fileManager.getCurrentFileName();
        string modifiedFlag;
        if (fileManager.hasUnsavedChanges())
            modifiedFlag = "[+]";
        else
            modifiedFlag = "";

        string statusLine = "" + modeText + " " + fileName + " " + modifiedFlag + " | Line: " +
            to_string(status.cursorLine) + ", Col: " +
            to_string(status.cursorColumn) + " | Total Lines: " +
            to_string(status.totalLines);
        return statusLine;
    }

    // display
    void display() const {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
        cout << "-----------------\n";
        bool cursorPrinted = false;
        for (int i = 0; i < lines.size(); ++i) {
            if (i == currentLine) {
                if (charCursor == nullptr) {
                    cout << "|";
                }
                cursorPrinted = lines[i].printLine(charCursor, cursorPrinted);
            }
            else {
                cursorPrinted = lines[i].printLine(nullptr, cursorPrinted);
            }
            cout << endl;
        }

        cout << "-----------------\n";
        cout << getStatusLineText() << endl;
    }

};

int getChar() {
#ifdef _WIN32
    int ch = _getch();
    if (ch == 0 || ch == 224) {
        ch = _getch();
        switch (ch) {
        case 72: return 1001;
        case 80: return 1002;
        case 77: return 1003;
        case 75: return 1004;
        }
    }
    return ch;
#else
    struct termios old_tio, new_tio;
    int c;
    tcgetattr(STDIN_FILENO, &old_tio);
    new_tio = old_tio;
    new_tio.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
    c = getchar();
    if (c == 27) {
        c = getchar();
        if (c == 91) {
            c = getchar();
            switch (c) {
            case 65: c = 65; break;
            case 66: c = 66; break;
            case 67: c = 67; break;
            case 68: c = 68; break;
            }
        }
        else {
            ungetc(c, stdin);
            c = 27;
        }
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
    return c;
#endif
}


int main() {
    TextEditor editor;
    int command;
    int nextCommand;
    bool ddFlag = false;
    bool yyFlag = false;

    while (true) {
        editor.updateStatusLine();
        editor.display();
        command = getChar();
        if (command == 27) {
            editor.exitInsertMode();
            continue;
        }
        if (!editor.isInsertMode()) {
            string commandBuffer;
            //  ':' commands
            if (command == ':') {
                cout << ":";
                getline(cin, commandBuffer);

                //  "d N" command to delete line N
                if (commandBuffer.rfind("d ", 0) == 0) { // Check if command starts with "d "
                    size_t lineNum = stoi(commandBuffer.substr(2));
                    editor.deleteLineNumber(lineNum);
                }
                //  replace commands
                else if (commandBuffer.rfind("s/", 0) == 0) { 
                    size_t firstSlash = 2;
                    size_t secondSlash = commandBuffer.find('/', firstSlash);
                    bool global = commandBuffer.find("/g") != std::string::npos;
                    size_t thirdSlash = commandBuffer.find('/', secondSlash + 1);

                    if (secondSlash != std::string::npos) {
                        std::string old = commandBuffer.substr(firstSlash, secondSlash - firstSlash);
                        string newStr;
                        if (global)
                            newStr = commandBuffer.substr(secondSlash + 1, thirdSlash - secondSlash - 1);
                        else
                            newStr = commandBuffer.substr(secondSlash + 1);

                        editor.replace(old, newStr, global);
                    }
                }
                else {
                    editor.handleFileCommand(commandBuffer);
                }
            }

            //  '/' search commands
            if (command == '/') {
                cout << "/";
                getline(cin, commandBuffer);
                if (editor.search(commandBuffer)) {
                    bool searchActive = true;
                    while (searchActive) {
                        editor.updateStatusLine();
                        editor.display();
                        nextCommand = getChar();
                        if (nextCommand == 'n') {
                            searchActive = editor.findNext();
                        }
                        else if (nextCommand == 'N') {
                            searchActive = editor.findPrevious();
                        }
                        else {
                            searchActive = false;
                            command = nextCommand;
                        }
                    }
                }
            }
        }

        //insert mode
        if (editor.isInsertMode()) {
            if (command == '\n' || command == '\r')
                editor.newLine();
            else {
                switch (command) {
                case 1001:
                    editor.moveUp();
                    break;
                case 1002:
                    editor.moveDown();
                    break;
                case 1003:
                    editor.moveRight();
                    break;
                case 1004:
                    editor.moveLeft();
                    break;
                case 8:
                    editor.deleteChar();
                    break;
                default:
                    editor.insertChar(static_cast<char>(command));
                    break;
                }
            }
        }
        // normal mode
        else {
            switch (command) {
            case 'i':
                editor.enterInsertMode();
                break;
            case 'x':
                editor.deleteChar();
                break;
            case 'D':
                editor.deleteFromCursorToEnd();
                break;
            case 'd':
                if (ddFlag) {
                    editor.deleteCurrentLine();
                    ddFlag = false;
                }
                else {
                    ddFlag = true;
                }
                yyFlag = false;
                break;
            case '0':
                editor.moveToStartOfLine();
                ddFlag = false;
                yyFlag = false;
                break;
            case '$':
                editor.moveToEndOfLine();
                ddFlag = false;
                yyFlag = false;
                break;
            case 'w':
                editor.moveToNextWord();
                ddFlag = false;
                yyFlag = false;
                break;
            case 'b':
                editor.moveToPreviousWord();
                ddFlag = false;
                yyFlag = false;
                break;
            case 'e':
                editor.moveToWordEnd();
                ddFlag = false;
                yyFlag = false;
                break;
            case 'y':
                if (yyFlag) {
                    editor.yankLine();
                    yyFlag = false;
                }
                else {
                    yyFlag = true;
                }
                ddFlag = false;
                break;
            case 'p':
                editor.pasteAfter();
                ddFlag = false;
                yyFlag = false;
                break;
            case 'P':
                editor.pasteBefore();
                ddFlag = false;
                yyFlag = false;
                break;
            case 1001:
                editor.moveUp();
                ddFlag = false;
                yyFlag = false;
                break;
            case 1002:
                editor.moveDown();
                ddFlag = false;
                yyFlag = false;
                break;
            case 1003:
                editor.moveRight();
                ddFlag = false;
                yyFlag = false;
                break;
            case 1004:
                editor.moveLeft();
                ddFlag = false;
                yyFlag = false;
                break;
            case 'n':
                editor.newLine();
                ddFlag = false;
                yyFlag = false;
                break;
            case 'J':
                editor.joinLines();
                break;
            case '>':
                nextCommand = getChar();
                if (nextCommand == '>')
                {
                    editor.indentLine(true);
                }
                break;
            case '<':
                nextCommand = getChar();
                if (nextCommand == '<')
                {
                    editor.indentLine(false);
                }
                break;
            case 'q':
                return 0;
            default:
                ddFlag = false;
                yyFlag = false;
                break;
            }
        }
    }
    return 0;
}
