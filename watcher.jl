using FileWatching

mutable struct Watcher
    last_filename::String 
    download_filenames::Set{String}
    chrome_download_regex::Regex

    function Watcher()
        return new("", Set{String}(), r".*\.crdownload$")
    end
end

function get_next_download_internal(watcher)
    while true
        change = watch_folder("/mnt/ram/")

        filename = change[1]
        renamed = change[2].renamed
        changed = change[2].changed
        timedout = change[2].timedout

        if !isnothing(match(watcher.chrome_download_regex, filename))
            if renamed
                if filename in watcher.download_filenames
                    println("Identified ", watcher.last_filename)
                    delete!(watcher.download_filenames, filename)
                    return watcher.last_filename
                else
                    push!(watcher.download_filenames, filename)
                end
            end
        else
            watcher.last_filename = filename
        end

    end
end

watcher = Watcher()

function get_next_download()
    return Vector{UInt8}(get_next_download_internal(watcher))
end
