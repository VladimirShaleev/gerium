#ifndef ASYNC_LOADER_HPP
#define ASYNC_LOADER_HPP

#include "Common.hpp"

class AsyncLoader final {
public:
    ~AsyncLoader();

    void create(gerium_application_t application, gerium_renderer_t renderer);
    void destroy();

    void loadTexture(gerium_texture_h texture, gerium_file_t file, gerium_cdata_t data);

private:
    struct Task {
        gerium_texture_h texture;
        gerium_file_t file;
        gerium_cdata_t data;
        gerium_cdata_t image;
    };

    void loader() noexcept;

    std::atomic<gerium_renderer_t> _renderer;

    gerium_signal_t _shutdownSignal;
    gerium_signal_t _waitTaskSignal;
    gerium_mutex_t _mutex;

    std::queue<Task*> _tasks;
};

#endif
