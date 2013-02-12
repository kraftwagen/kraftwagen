# Kraftwagen

Kraftwagen is Drush extensions that provides a set of comands to support a
_everything in code_ and _install profile_/`drush make` based Drupal development
workflow.

## Directory structure

* `archive`: Directory where old builds will be stored when you do not have a
  `builds` dir.
* `build`: Directory that contains the working Drupal installation or a symlink
  to the working Drupal installation, in a subdir of `builds`.
* `builds`: Optional directory in which multiple builds can be stored.
* `cnf`: Contains settings, the Drupal upload directory and the environment
  file. The settings and files directory will be symlinked into the `build`.
* `src`: The actual source of the project, which is a Drupal install profile.
* `tools`: Optional directory in which you can have a Drush and Kraftwagen per
  project. You can setup this directory using our
  [tools repository](https://github.com/kraftwagen/tools).

## Available commands

Kraftwagen provides a set of commands. Some of these commands are public, and
some are hidden. All of the hidden commands are called by public commands. The
hidden commands are there to allow Kraftwagen to work with automated deployment
tools like [Capistrano](http://capistranorb.com/).

* `drush kw-new-project`: Creates a `src` directory in which the source of a the
  new project is placed, based on a provided skeleton Git repository. Although
  it is easily possible to create a new project from scratch without using this
  command, this will get you up to speed more quickly and allows for a
  company-wide standard on how Drupal projects are constructed (like modules
  that always should be enabled or desired default configuration). We provide a
  [skeleton example](https://github.com/kraftwagen/skeleton) that you can fork
  or copy for your own work.
* `drush kw-setup`: Creates a `cnf` directory in which a few files and a
  directory are created. The directory is called `files` and is intended to
  become the directory in which the user uploaded files in Drupal are stored.
  The files are called `environment`, `settings.php` and `settings.local.php`.
  The `environment` file contains the type of enviroment, like 'production'
  (which is the default), 'staging', 'testing' or 'development'. The
  `settings.php` and `settings.local.php` files are copied from the `src/cnf`
  directory. `settings.php` should include `settings.local.php` (in our
  skeleton example this is setup correctly), so all global settings can be in
  `settings.php` and settings that differ per deployment, like database
  credentials, can be in `settings.local.php`.
* `drush kw-update-makefile`: Usually, updating Drupal installations to new
  versions can be real pain the bottom. With Kraftwagen, all core and contrib
  projects are saved by just the version number. Everything else will be
  downloaded when building. This make updating already a breeze, because it will
  be as simple as updating the number in your `.make` file and rebuilding. It
  can be even more easy. This command accepts the location of a `.make` file and
  will automatically update all version numbers for you. *It is recommended to
  keep the file in version control, because this command will change the file,
  so you want a rollback.*
* `drush kw-build`: This command creates a `build` directory that contains a
  runnable Drupal installation. It will use `src/tools/build.make.tpl` to
  construct a make file and then will feed that file to `drush make` to create
  the installation. When `drush make` is finished, it will symlink the
  `cnf/files` directory and the `cnf/settings.php` and `cnf/settings.local.php`
  into `build/sites/default`. If you created a `builds` directory, it will not
  create the build in `build`. It will instead create the build in datestamped
  subdirectory of `builds` and make `build` a symlink to this subdirectory.
* `drush kw-init-database`: This command is fairly simple wrapper around
  `drush site-install`. The wrapper is just that it will automatically find out
  the name of the install profile that should be used. It can only be ran in the
  build.
* `drush kw-update`: This command makes sure all modules that should be enabled,
  are enabled, and that modules that should be disable, are disable. Then it
  runs all pending database updates for modules (`drush updatedb`). It's third
  step is reverting all features (`drush features-revert-all`) and at last it
  will run all
  [Kraftwagen Manifests](https://github.com/kraftwagen/kw-manifests). This
  command can only be ran in the build.

