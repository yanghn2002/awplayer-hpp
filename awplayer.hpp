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
        virtual void stop(void) = 0;

};


struct DLL {

    void* _dll;

    DLL(const char* dll): _dll(nullptr) {
    
        _dll = ::dlopen(dll, RTLD_LAZY);
        if(!_dll) std::runtime_error("dlopen");
    
    }

    virtual ~DLL(void) {

        ::dlclose(_dll);

    }

    protected:

        template<typename SYM_T>
        SYM_T _load_symbol(const char* symbol) {
            if(_dll) {
                SYM_T ptr = reinterpret_cast<SYM_T>(::dlsym(_dll, symbol));
                if(ptr) return ptr;
                else std::runtime_error("dlsym");
            } else return nullptr;
        }

        virtual void load_symbols(void) = 0;

};


class TPlayer final: public AWPlayer {
    
    public:

        struct TPlayerDLL final: protected DLL {

            using NotifyCallback       = int(*)(void*, int, int, void*);
                
            using APICreate            = void*(*)(int);
            using APISetNotifyCallback = int(*)(void*, NotifyCallback, void*);
            using APISetDataSource     = int(*)(void*, const char*, void*);
            using APIPrepare           = int(*)(void*);
            using APIStart             = int(*)(void*);
            using APISetDisplayRect    = void(*)(void*, int, int, uint32_t, uint32_t);
            using APIStop              = int(*)(void*);
            using APIDestroy           = void(*)(void*);
                
            APICreate            apiCreate;
            APISetNotifyCallback apiSetNotifyCallback;
            APISetDataSource     apiSetDataSource;
            APIPrepare           apiPrepare;
            APIStart             apiStart;
            APISetDisplayRect    apiSetDisplayRect;
            APIStop              apiStop;
            APIDestroy           apiDestroy;

            explicit TPlayerDLL(const char* dll): apiCreate(nullptr),
                                                  apiSetNotifyCallback(nullptr),
                                                  apiSetDataSource(nullptr),
                                                  apiPrepare(nullptr),
                                                  apiStart(nullptr),
                                                  apiSetDisplayRect(nullptr),
                                                  apiStop(nullptr),
                                                  apiDestroy(nullptr),
                                                  DLL(dll) { load_symbols(); }

            ~TPlayerDLL(void) override = default;

            private:

                virtual void load_symbols(void) override {
                    
                    apiCreate            = _load_symbol<APICreate>
                                                       ("TPlayerCreate");
                    apiSetNotifyCallback = _load_symbol<APISetNotifyCallback>
                                                       ("TPlayerSetNotifyCallback");
                    apiSetDataSource     = _load_symbol<APISetDataSource>
                                                       ("TPlayerSetDataSource");
                    apiPrepare           = _load_symbol<APIPrepare>
                                                       ("TPlayerPrepare");
                    apiStart             = _load_symbol<APIStart>
                                                       ("TPlayerStart");
                    apiSetDisplayRect    = _load_symbol<APISetDisplayRect>
                                                       ("TPlayerSetDisplayRect");
                    apiStop              = _load_symbol<APIStop>
                                                       ("TPlayerStop");
                    apiDestroy           = _load_symbol<APIDestroy>
                                                       ("TPlayerDestroy");
                
                }

        };

    private:

        TPlayerDLL _dll;
        void* _player;

    public:
    
        explicit TPlayer (
            const TPlayerDLL::NotifyCallback ncb=[](void* _a0, int  _a1, int _a2, void* _a3)
                { (void)_a0; (void)_a1; (void)_a2; (void)_a3; return 0; }
        ): _dll(TPLAYER_DLL) {
            
            _player = _dll.apiCreate(0);
            if(!_player) throw std::runtime_error("TPlayerCreate");

            if(_dll.apiSetNotifyCallback(_player, ncb, reinterpret_cast<void*>(this)))
                throw std::runtime_error("TPlayerSetNotifyCallback");

        }

        virtual void play(const std::string& url) override {

            if(_player) {

                if(_dll.apiSetDataSource(_player, url.c_str(), NULL))
                    throw std::runtime_error("TPlayerSetDataSource");

                if(_dll.apiPrepare(_player))
                    throw std::runtime_error("TPlayerPrepare");

                if(_dll.apiStart(_player))
                    throw std::runtime_error("TPlayerStart");

            }

        }

        virtual void stop(void) override {

            _dll.apiStop(_player);

        }

        void operator()(const int x ,const int y, const uint32_t w, const uint32_t h) const {

            _dll.apiSetDisplayRect(_player, x, y, w, h);

        }
        
        ~TPlayer(void) override {

            stop();
            _dll.apiDestroy(_player);

        };

};


}


#endif//AWPLAYER_HPP