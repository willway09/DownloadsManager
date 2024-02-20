using FileWatching

mutable struct FileState
    handler::Any
    sequence::Any
end

mutable struct Watcher
    last_filename::String
    files::Dict{String, FileState}

    function Watcher()
        return new("", Dict{String, FileState}())
    end
end

function decompose_change(change)
    return (change[1], change[2].renamed, change[2].changed, change[2].timedout)
end

function chrome_download_handler(change, state::FileState)
    (filename, renamed, changed, timedout) = decompose_change(change)

    if state == 4
        return 2
    end

    if renamed
        if state.sequence == 0
            state.sequence += 1
        elseif state.sequence == 1
            state.sequence += 1
            return 1 # Claim last file
        elseif state.sequence == 2
            state.sequence += 1
            # Ignore this
        elseif state.sequence == 3
            state.sequence += 1
            return 1 # Claim last file
        else
            return 2 # Finally have filename
        end
    end
    return 0
end

function old_chrome_download_handler(change, state::FileState)
    (filename, renamed, changed, timedout) = decompose_change(change)

#    if !isnothing(match(watcher.chrome_download_regex, filename))
#        if renamed
#            if filename in watcher.download_filenames
#                if(log)
#                    # println("Identified ", watcher.last_filename)
#                end
#                delete!(watcher.download_filenames, filename)
#                return watcher.last_filename
#            else
#                push!(watcher.download_filenames, filename)
#            end
#        end
#    else
#        watcher.last_filename = filename
#    end

    if(state.sequence == 0)
        state.sequence += 1
    end
end

function claim_last_filename(files, filename, last_filename)
    #println("Renaming $(filename) to $(last_filename)")
    files[last_filename] = files[filename]
    pop!(files, filename)
end

function finalize(files, filename)
        pop!(files, filename)
end

function get_next_download_internal(watcher, log)
    # Declare regexes for common patterns
    old_chrome_download_regex = r".*\.crdownload$"
    chrome_download_regex =  r"^\.org\.chromium\.Chromium\..*"

    if(log)
        println("Filename: $(filename) | Renamed: $(renamed) | Changed: $(changed) | Timedout: $(timedout)")
    end

    while true
        change = watch_folder("/mnt/ram/")

        filename = change[1]
        renamed = change[2].renamed
        changed = change[2].changed
        timedout = change[2].timedout

        if(haskey(watcher.files, filename))
            state = watcher.files[filename]
            res = state.handler(change, state)
            if res == 1
                claim_last_filename(watcher.files, filename, watcher.last_filename)
            elseif res == 2
                finalize(watcher.files, filename)
                return filename
            end
        elseif !isnothing(match(chrome_download_regex, filename))
            watcher.files[filename] = FileState(chrome_download_handler, 0)
            watcher.files[filename].handler(change, watcher.files[filename])
        elseif !isnothing(match(old_chrome_download_regex, filename))
            watcher.files[filename] = FileState(old_chrome_download_handler, 0)
            watcher.files[filename].handler(change, watcher.files[filename])
        end

        watcher.last_filename = filename

    end
end

watcher = Watcher()

function get_next_download()
    return Vector{UInt8}(get_next_download_internal(watcher, false))
end

function test()
    while(true)
        println("Identified: ", get_next_download_internal(watcher, true))
    end
end

test()
