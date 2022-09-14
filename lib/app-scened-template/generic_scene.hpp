template <typename TApp>
class GenericScene {
public:
    virtual void on_enter(TApp* app, bool need_restore) = 0;
    virtual bool on_event(TApp* app, typename TApp::Event* event) = 0;
    virtual void on_exit(TApp* app) = 0;
    virtual ~GenericScene(){};

private:
};
