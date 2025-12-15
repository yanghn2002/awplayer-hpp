#ifndef AWPLAYER_HPP
#define AWPLAYER_HPP


#ifndef TPLAYER_DLL
    #define TPLAYER_DLL "libtplayer.so"
#endif


#include <string>
#include <stdexcept>
#include <dlfcn.h>


namespace AWPlayer {


enum class Type {
    TPlayer,
};


class AWPlayer {

    public:

        AWPlayer(void) = default;
        virtual ~AWPlayer(void) = default;
        virtual void play(const std::string&) = 0;
        virtual void pause(void) = 0;
        virtual void reset(void) = 0;
        virtual void stop(void) = 0;
        virtual int state(void) = 0;

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

        enum State {
            _       = 0,
            PLAYING = 1,
        };

    private:

        TPlayerDLL _dll;
        void* _player;

    public:
    
        explicit TPlayer (
            const TPlayerDLL::NotifyCallback ncb=[](void* _a0, int  _a1, int _a2, void* _a3)
                { (void)_a0; (void)_a1; (void)_a2; (void)_a3; return 0; }
        ): _dll(TPLAYER_DLL), _player(nullptr) {
            
            if(_dll) _player = _dll.apiCreate(0);

            if(_player) {
                if(_dll.apiSetNotifyCallback)
                    if(_dll.apiSetNotifyCallback(_player, ncb, reinterpret_cast<void*>(this)))
                        throw std::runtime_error("TPlayerSetNotifyCallback");
            } else throw std::runtime_error("TPlayerCreate");

        }

        virtual void play(const std::string& url) override {

            if(_dll.apiSetDataSource)
                if(_dll.apiSetDataSource(_player, url.c_str(), NULL))
                    throw std::runtime_error("TPlayerSetDataSource");

            if(_dll.apiPrepare)
                if(_dll.apiPrepare(_player))
                    throw std::runtime_error("TPlayerPrepare");

            if(_dll.apiStart)
                if(_dll.apiStart(_player))
                    throw std::runtime_error("TPlayerStart");

        }

        virtual void pause(void) override {

            if(_dll.apiPause) _dll.apiPause(_player);
        
        }

        virtual void reset(void) override {

            if(_dll.apiReset) _dll.apiReset(_player);
        
        }

        virtual void stop(void) override {

            if(_dll.apiStop) _dll.apiStop(_player);
        
        }

        virtual int state(void) override {

            return _dll.apiIsPlaying(_player) ? PLAYING : _;

        }

        void operator()(const int x ,const int y, const uint32_t w, const uint32_t h) const {

            if(_dll.apiSetDisplayRect) _dll.apiSetDisplayRect(_player, x, y, w, h);

        }
        
        ~TPlayer(void) override {

            stop();
            
            if(_dll.apiDestroy) _dll.apiDestroy(_player);

        };

};


}


#endif//AWPLAYER_HPP