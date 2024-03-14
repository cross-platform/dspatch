project('DSPatch', 'cpp',
    default_options: [
        'cpp_std=c++17',
        'warning_level=3',
        'werror=true'
    ]
)

if host_machine.system() == 'darwin'
    env = environment()
    env.set('MACOSX_DEPLOYMENT_TARGET', '10.13')

    add_project_arguments(['-mmacosx-version-min=10.13', '-arch', 'arm64', '-arch', 'x86_64'], language: 'cpp')
    add_project_link_arguments(['-mmacosx-version-min=10.13', '-arch', 'arm64', '-arch', 'x86_64'], language: 'cpp')
endif

# Add code coverage

opencppcoverage = find_program('opencppcoverage', required: false)

opencppcoverage_args = [
    '--sources', meson.project_name(),
    '--excluded_sources', 'tests',
    '--excluded_sources', 'tutorial'
]

if opencppcoverage.found()
    run_target('codecoverage',
        command: [
            opencppcoverage,
            '--input_coverage', 'Tests.cov',
            '--export_type', 'html:coverage'
        ]
    )
endif

# Add formatting

format_first = []
if find_program('clang-format', required: false).found()
    if host_machine.system() == 'windows'
        format_command = meson.current_source_dir() + '/scripts/clang-format.bat'
    else
        format_command = ['bash', meson.current_source_dir() + '/scripts/clang-format.sh']
    endif

    format_first = custom_target(
        output: 'formatting',
        command: format_command,
        build_always: true
    )
endif

# Add static code analysis

if find_program('cppcheck', required: false).found()
    if host_machine.system() == 'windows'
        extra_cppcheck_args = ['-D_WIN32']
    else
        extra_cppcheck_args = []
    endif

    custom_target(
        input: format_first,
        output: 'static code analysis',
        command: [
            'cppcheck',

            '--error-exitcode=1',
            '--enable=all',
            '--inconclusive',
            '--inline-suppr',
            '--force',
            '--quiet',
            '--suppress=missingIncludeSystem',
            '--suppress=*:' + meson.current_source_dir() + '/include/fast_any/any.h',

            '-i', meson.current_source_dir() + '/build',
            '-I', meson.current_source_dir() + '/include',

            extra_cppcheck_args,

            meson.current_source_dir()
        ],
        build_always: true
    )
endif

# Configure DSPatch

dspatch_dep = declare_dependency(
  include_directories: include_directories('include'),
  dependencies: dependency('threads')
)

# Add subdirectories

subdir('tests')
subdir('tutorial')
