#include <gtkmm.h>
#include <iostream>
#include <fstream>


class MainWindow : public Gtk::Window
{
public:
    MainWindow();

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


    void onFileSelectionChanged();

    void loadFile(const std::string& filePath);

    public:
    void populateFileExplorer(const std::string& directoryPath);
};

MainWindow::MainWindow()
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
    // fileScrolledWindow.override_background_color(Gdk::RGBA("lightgray")); // Set background color to light gray
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
}


void MainWindow::populateFileExplorer(const std::string& directoryPath)
{
    // Glib::RefPtr<Gtk::TreeStore> treeStore = Gtk::TreeStore::create(columns);

    // Get a list of files in the directory
    Glib::Dir dir(directoryPath);
    std::string filename;
    while (!(filename = dir.read_name()).empty())
    {
        Gtk::TreeModel::Row row = *(fileTreeStore->append());
        row[columns.filename] = filename;
        std::cout<<filename<<std::endl;
    }
    show();
    // fileTreeView.set_model(treeStore);
    // fileScrolledWindow.
}


void MainWindow::onFileSelectionChanged()
{
    Gtk::TreeModel::iterator iter = fileTreeView.get_selection()->get_selected();
    if (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        std::string filename = row[columns.filename];
        std::string filePath = "./" + filename; // Update with the actual directory path
        std::cout<<filePath<<std::endl;
        loadFile(filePath);
    }
}

void MainWindow::loadFile(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (file.is_open())
    {
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        std::cout<<"opening"<<std::endl;
        textBuffer->set_text(content);
    }
    else
    {
        std::cerr << "Failed to open file: " << filePath << std::endl;
    }
    show();
}

int main(int argc, char* argv[])
{
    auto app = Gtk::Application::create("org.gtkmm.examples.base");
    MainWindow mainWindow;
    mainWindow.populateFileExplorer("/home/akshar");
    // Run the application with your custom window
    return app->make_window_and_run<MainWindow>(argc, argv);

}
