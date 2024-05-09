#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <limits>
#include <gtkmm.h>

using namespace std;
char buffer[1024] = {0};

int first=0;
class MainWindow : public Gtk::Window
{
public:
    MainWindow();
    virtual ~MainWindow();
protected:
    Gtk::Box mainBox;
    Gtk::Box fileExplorerBox;
    Gtk::TreeView fileTreeView;
    Gtk::ScrolledWindow fileScrolledWindow;
    Gtk::TextView textEditorTextView;
    Glib::RefPtr<Gtk::TextBuffer> textBuffer;
    Glib::RefPtr<Gtk::TreeStore> fileTreeStore;
    struct Columns : public Gtk::TreeModel::ColumnRecord
    {
        Columns()
        {
            add(filename);
        }

        Gtk::TreeModelColumn<std::string> filename;
    } columns;

    int clientSocket;

    void onFileSelectionChanged();

    void loadFile(const std::string& filePath);

    void connectToServer();
     void onTextChanged();

    void saveAndSendFile(const std::string& filePath);

    public:
    void populateFileExplorer(const std::string& directoryPath);
    bool on_delete_event(GdkEventAny* event) override;

    // Method to close the connection with the server
    void closeConnection();    
};

MainWindow::~MainWindow() {
    // Close the connection with the server when the window is destroyed
    //cout<<"in destructior"<<endl;
    closeConnection();
}

bool MainWindow::on_delete_event(GdkEventAny* event) {
    // Close the connection with the server when the window is closed
    //cout<<"in delete event\n";
    closeConnection();
    return Gtk::Window::on_delete_event(event);
}

void MainWindow::closeConnection() {
    // Send a specific message to the server indicating that the client is closing
    const char* closeMessage = "CLIENT_CLOSE\n";
    send(clientSocket, closeMessage, strlen(closeMessage), 0);
    sleep(0.5);
    // Close the client socket
    close(clientSocket);
}

MainWindow::MainWindow() : clientSocket(-1)
{
    set_title("GOOGLE DOCS");
    set_default_size(800, 600);

    // Set up the main box
    set_child(mainBox);

    // Set up the file explorer pane
    fileTreeStore = Gtk::TreeStore::create(columns);
    fileTreeView.set_model(fileTreeStore);
    fileTreeView.append_column("Files", columns.filename);

    // Set up the scrolled window
    fileScrolledWindow.set_child(fileTreeView);
    fileScrolledWindow.set_size_request(250, -1); // Set the width to 250 pixels

    fileExplorerBox.append(fileScrolledWindow);
    mainBox.append(fileExplorerBox);

    // Set up the text editor pane
    textBuffer = Gtk::TextBuffer::create();
    textEditorTextView.set_buffer(textBuffer);
    textEditorTextView.set_size_request(550,1);
    mainBox.append(textEditorTextView);

    // Connect signals
    fileTreeView.signal_cursor_changed().connect(sigc::mem_fun(*this, &MainWindow::onFileSelectionChanged));
    textBuffer->signal_changed().connect(sigc::mem_fun(*this, &MainWindow::onTextChanged));

    // Connect to server
    connectToServer();
}

void MainWindow::populateFileExplorer(const std::string& directoryPath)
{
    if (clientSocket < 0) {
        cerr << "Error: Not connected to server\n";
        return;
    }

    // Send request to server to get list of files
    // if(first==0) {
    //     first++;
    //     return;
    // }
    const char *message = "GET_FILES\n";
    cout<<"*****************************************************************"<<endl;
    cout<<*message<<" "<<strlen(message)<<endl;;
    int bytesSent = send(clientSocket, message, strlen(message), 0);
    if (bytesSent < 0) {
        cerr << "Error: Failed to send data to server\n";
        return;
    }
    // sleep(0.5);
    // Receive file list from server
    char buffer[1024] = {0};
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead < 0) {
        cerr << "Error: Failed to receive data from server\n";
        return;
    }

    // Update file tree view with received file list
    std::string fileData(buffer, bytesRead);
    std::istringstream iss(fileData);
    std::string filename;
    while (std::getline(iss, filename, '\n')) {
        Gtk::TreeModel::Row row = *(fileTreeStore->append());
        row[columns.filename] = filename;
    }

    //show();
}

void MainWindow::onFileSelectionChanged()
{
    Gtk::TreeModel::iterator iter = fileTreeView.get_selection()->get_selected();
    if (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        std::string filename = row[columns.filename];
        std::string filePath = filename;
        cout<<"first"<<endl;
        loadFile(filePath);
    }
}

void MainWindow::loadFile(const std::string& filePath)
{
    if (clientSocket < 0) {
        cerr << "Error: Not connected to server\n";
        return;
    }
    
    // Send request to server to get file content
    std::cout<<filePath<<std::endl;
    std::string message = "GET_FILE\n" + filePath + "\n";
    int bytesSent = send(clientSocket, message.c_str(), message.size(), 0);
    sleep(0.5);
    if (bytesSent < 0) {
        cerr << "Error: Failed to send data to server\n";
        return;
    }
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    //std::cout<<"recv "<<buffer<<std::endl;

    if (bytesRead < 0) {
        cerr << "Error: Failed to receive data from server\n";
        return;
    }
    std::string response(buffer,bytesRead);
    cout<<"response = "<<response<<endl;
    size_t start = response.find("RESPONSE");
    size_t end = response.find("END");
    if(start!=std::string::npos && end != std::string::npos){
        std::string content = response.substr(start+strlen("RESPONSE"),end - start-strlen("RESPONSE"));
        textBuffer->set_text(content);
    }
    else{
        cerr<<"error from reading\n";
    }
    // Update text buffer with received file content
    
    // memset(buffer, 0, sizeof(buffer));

    
    
}

void MainWindow::saveAndSendFile(const std::string& filePath)
{
    if (clientSocket < 0) {
        cerr << "Error: Not connected to server\n";
        return;
    }

    // Get the content of the text buffer
    Gtk::TextBuffer::iterator start = textBuffer->begin();
    Gtk::TextBuffer::iterator end = textBuffer->end();
    std::string content = textBuffer->get_text(start, end);
    std::cout<<"sending update file path ="<<filePath<<endl;
    // Send the file content to the server
    std::string message = "UPDATE_FILE\n" + filePath + "\n" + content;
    int bytesSent = send(clientSocket, message.c_str(), message.size(), 0);
    if (bytesSent < 0) {
        cerr << "Error: Failed to send data to server\n";
        return;
    }
}

void MainWindow::onTextChanged()
{
    // Get the currently selected file from the file tree view
    Gtk::TreeModel::iterator iter = fileTreeView.get_selection()->get_selected();
    std::cout<<"on text selected"<<endl;
    if (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        std::string filename = row[columns.filename];
        std::string filePath = filename;
        saveAndSendFile(filePath);
    }
}



void MainWindow::connectToServer()
{
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        cerr << "Error: Failed to create socket\n";
        return;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Error: Failed to connect to server\n";
        // close(clientSocket);
        return;
    }
    
    populateFileExplorer("./");
}

int main(int argc, char* argv[])
{
    auto app = Gtk::Application::create("org.gtkmm.examples.base");
    MainWindow mainWindow;
    // mainWindow.connectToServer();
    return app->make_window_and_run<MainWindow>(argc, argv);
}
