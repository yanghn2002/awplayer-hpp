#ifndef AWPLAYER_HPP
#define AWPLAYER_HPP


#ifndef TPLAYER_DLL
    #define TPLAYER_DLL "libtplayer.so"
#endif


#include <map>
#include <string>
#include <stdexcept>
#include <dlfcn.h>


namespace AWPlayer {


enum State {
    ERROR    =-1,
    IDLE     = 0,
    COMPLETE = 1,
    STOPED   = 2,
    PLAYING  = 3,
    PREPARED = 4,
    PAUSED   = 5,
};

enum class Type {
    TPlayer,
};


class AWPlayer {

    protected:

        State _state;

        virtual void _prepare(void) = 0;
        virtual void _start(void) = 0;
        virtual void _pause(void) = 0;
        virtual void _reset(void) = 0;
        virtual void _stop(void) = 0;

    public:

        AWPlayer(void): _state(State::IDLE) { }
        virtual ~AWPlayer(void) = default;

        State state(void) {

            return _state;

        }

        bool isOk(void) {

            return static_cast<int>(_state) >= 0 ? true : false;

        }

        bool isWorking(void) {

            return static_cast<int>(_state) >= 3 ? true : false;

        }

        virtual void setVideo(const std::string&) = 0;
        virtual void setDisplayRect(const int, const int, const uint32_t, const uint32_t) = 0;

        void prepare(void) {

            _prepare();
            _state = State::PREPARED;

        }

        void start(void) {

            _start();
            _state = State::PLAYING;

        }

        void pause(void) {

            _pause();
            _state = State::PAUSED;

        }

        void reset(void) {

            _reset();
            _state = State::IDLE;

        }

        void stop(void) {

            _stop();
            _state = State::STOPED;

        }
        
        void play(const std::string& url) {

            setVideo(url);
            prepare();
            start();

        }

};


struct DLL {

    void* _dll;

    DLL(const char* dll): _dll(nullptr) {
    
        _dll = ::dlopen(dll, RTLD_LAZY);
        if(!_dll) throw std::runtime_error("dlopen");
    
    }

    virtual ~DLL(void) {

        ::dlclose(_dll);

    }

    operator bool() const noexcept {

        return _bool();

    }

    private:

        bool _bool(void) const noexcept {

            return _dll == nullptr ? false : true;

        }

    protected:

        template<typename SYM_T>
        SYM_T _load_symbol(const char* symbol) {
            if(_bool()) {
                SYM_T ptr = reinterpret_cast<SYM_T>(::dlsym(_dll, symbol));
                if(ptr) return ptr;
                else throw std::runtime_error("dlsym");
            } else return nullptr;
        }

        virtual void load_symbols(void) = 0;

};


class TPlayer final: public AWPlayer {
    
    public:

        struct TPlayerDLL final: DLL {

            using NotifyCallback       = int(*)(void*, int, int, void*);
                
            using APICreate            = void*(*)(int);
            using APISetDataSource     = int(*)(void*, const char*, void*);
            using APIPrepare           = int(*)(void*);
            using APIStart             = int(*)(void*);
            using APIPause             = int(*)(void*);
            using APIReset             = int(*)(void*);
            using APIStop              = int(*)(void*);
            using APIIsPlaying         = bool(*)(void*);
            using APIDestroy           = void(*)(void*);
            using APISetDisplayRect    = void(*)(void*, int, int, uint32_t, uint32_t);
            using APISetNotifyCallback = int(*)(void*, NotifyCallback, void*);
                
            APICreate            apiCreate;
            APISetDataSource     apiSetDataSource;
            APIPrepare           apiPrepare;
            APIStart             apiStart;
            APIPause             apiPause;
            APIReset             apiReset;
            APIStop              apiStop;
            APIIsPlaying         apiIsPlaying;
            APIDestroy           apiDestroy;
            APISetDisplayRect    apiSetDisplayRect;
            APISetNotifyCallback apiSetNotifyCallback;

            explicit TPlayerDLL(const char* dll): apiCreate(nullptr),
                                                  apiSetDataSource(nullptr),
                                                  apiPrepare(nullptr),
                                                  apiStart(nullptr),
                                                  apiPause(nullptr),
                                                  apiReset(nullptr),
                                                  apiStop(nullptr),
                                                  apiIsPlaying(nullptr),
                                                  apiDestroy(nullptr),
                                                  apiSetDisplayRect(nullptr),
                                                  apiSetNotifyCallback(nullptr),
                                                  DLL(dll) { load_symbols(); }

            ~TPlayerDLL(void) override = default;

            private:

                virtual void load_symbols(void) override {
                    
                    apiCreate            = _load_symbol<APICreate>
                                                       ("TPlayerCreate");
                    apiSetDataSource     = _load_symbol<APISetDataSource>
                                                       ("TPlayerSetDataSource");
                    apiPrepare           = _load_symbol<APIPrepare>
                                                       ("TPlayerPrepare");
                    apiStart             = _load_symbol<APIStart>
                                                       ("TPlayerStart");
                    apiPause             = _load_symbol<APIPause>
                                                       ("TPlayerPause");
                    apiReset             = _load_symbol<APIReset>
                                                       ("TPlayerReset");
                    apiStop              = _load_symbol<APIStop>
                                                       ("TPlayerStop");
                    apiIsPlaying         = _load_symbol<APIIsPlaying>
                                                       ("TPlayerIsPlaying");
                    apiDestroy           = _load_symbol<APIDestroy>
                                                       ("TPlayerDestroy");
                    apiSetDisplayRect    = _load_symbol<APISetDisplayRect>
                                                       ("TPlayerSetDisplayRect");
                    apiSetNotifyCallback = _load_symbol<APISetNotifyCallback>
                                                       ("TPlayerSetNotifyCallback");
                
                }

        };

    private:

        TPlayerDLL _dll;
        void* _player;

    public:
    
        explicit TPlayer(void): _dll(TPLAYER_DLL), _player(nullptr) {
            
            if(_dll) _player = _dll.apiCreate(0);

            if(_player) {
                if(_dll.apiSetNotifyCallback)
                    if(_dll.apiSetNotifyCallback(_player,
                            [](void* self, int msg, int param0, void* param1){
                                (int)param0; (void*)param1; // unused
                                if(msg ==  1)     ((TPlayer*)self)->_state = State::COMPLETE;
                                else if(msg == 3) ((TPlayer*)self)->_state = State::ERROR;
                                return 0;
                            }, this))
                        throw std::runtime_error("TPlayerSetNotifyCallback");
            } else throw std::runtime_error("TPlayerCreate");

        }

        virtual void setVideo(const std::string& url) override {

            if(_dll.apiSetDataSource)
                if(_dll.apiSetDataSource(_player, url.c_str(), NULL))
                    throw std::runtime_error("TPlayerSetDataSource");
        
        }

        virtual void setDisplayRect
        (const int x ,const int y, const uint32_t w, const uint32_t h) override {

            if(_dll.apiSetDisplayRect) _dll.apiSetDisplayRect(_player, x, y, w, h);

        }
        
        ~TPlayer(void) override {

            stop();
            
            if(_dll.apiDestroy) _dll.apiDestroy(_player);

        };
    
    private:

        virtual void _prepare(void) override {

            if(_dll.apiPrepare)
                if(_dll.apiPrepare(_player))
                    throw std::runtime_error("TPlayerPrepare");
        
        }

        virtual void _start(void) override {

            if(_dll.apiStart)
                if(_dll.apiStart(_player))
                    throw std::runtime_error("TPlayerStart");
        
        }

        virtual void _pause(void) override {

            if(_dll.apiPause)
                if(_dll.apiPause(_player))
                    throw std::runtime_error("TPlayerPause");
        
        }

        virtual void _reset(void) override {

            if(_dll.apiReset)
                if(_dll.apiReset(_player))
                    throw std::runtime_error("TPlayerReset");
        
        }

        virtual void _stop(void) override {

            if(_dll.apiStop)
                if(_dll.apiStop(_player))
                    throw std::runtime_error("TPlayerStop");
        
        }

};


}


#endif//AWPLAYER_HPP