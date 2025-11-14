set_project("espace-confiote")
set_version("1.0.0")

add_rules("mode.debug", "mode.release")

set_languages("c11")

add_requires("raylib 5.5")

option("ai_backend")
    set_values("c")
    set_description("Choose the player AI backend (c or python)")
option_end()

target("espace-confiote", function()
    set_kind("binary")

    local arch = get_config("arch") or "x64"
    local is_web = (arch == "wasm" or arch == "wasm32")

    add_packages("raylib")
    
    if is_web then
        add_defines("USE_PYTHON_AI", "PLATFORM_WEB")
        add_files("src/*.c", {excludes = {"src/player_ai_backend/*.c"}})
        add_files("src/player_ai_backend/player_ai_web.c")

        -- Only add as link flags, not compile flags
        add_ldflags("-sUSE_GLFW=3", {force = true})
        add_ldflags("-sHTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS=0", {force = true})
        add_ldflags("-sALLOW_MEMORY_GROWTH=1", {force = true})
        add_ldflags("-sASYNCIFY", {force = true})
        add_ldflags("-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap']", {force = true})
        add_ldflags("-sEXPORTED_FUNCTIONS=['_main','_malloc','_free','_web_player_shoot','_web_player_set_thrusters']", {force = true})
        add_ldflags("-O2", {force = true})

        -- Resource preloading during linking
        on_load(function (target)
            local projectdir = os.projectdir()
            local resources_path = path.join(projectdir, "resources")
            target:add("ldflags", "--embed-file " .. resources_path .. "@/resources", {force = true})
        end)

        before_link(function (target) -- Remove Windows system libraries for web
            local syslinks = target:get("syslinks") or {}
            local filtered = {}
            local win_libs = {"opengl32", "gdi32", "user32", "winmm", "shell32"}
            for _, lib in ipairs(syslinks) do
                local skip = false
                for _, win_lib in ipairs(win_libs) do
                    if lib == win_lib then
                        skip = true
                        break
                    end
                end
                if not skip then
                    table.insert(filtered, lib)
                end
            end
            target:set("syslinks", filtered)
        end)
        
        set_extension(".html")
        set_targetdir("$(projectdir)/build/web")
        
        after_build(function (target)
            -- Rename the generated HTML to a backup and use our custom one
            local target_html = "$(projectdir)/build/web/espace-confiote.html"
            local backup_html = "$(projectdir)/build/web/espace-confiote-emscripten.html"
            if os.isfile(target_html) then
                os.mv(target_html, backup_html)
            end
            if os.isfile("$(projectdir)/web/index.html") then
                os.cp("$(projectdir)/web/index.html", "$(projectdir)/build/web/")
            end
        end)
    else
        add_files("src/*.c", {excludes = {"src/player_ai_backend/*.c"}})
        
        local ai_backend = get_config("ai_backend")

        add_files("src/player_ai_backend/player_ai_c.c")
        add_defines("USE_C_AI")

        set_targetdir("$(projectdir)/build")
        
        after_build(function (target)
            os.cp("$(projectdir)/resources", "$(projectdir)/build/")
        end)
    end
end)