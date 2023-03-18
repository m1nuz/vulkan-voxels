include(FetchContent)

FetchContent_Declare(
    stb_image
    URL https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
    DOWNLOAD_NO_EXTRACT true
)

FetchContent_MakeAvailable(stb_image)