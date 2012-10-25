# Kraftwagen

Kraftwagen is Drush extensions that provides a set of comands to support a 
_everything in code_ and _install profile_/`drush make` based Drupal development
workflow.

## Available commands

Kraftwagen provides a set of commands. Some of these commands are public, and 
some are hidden. All of the hidden commands are called by public commands. The
hidden commands are there to allow Kraftwagen to work with automated deployment
tools like Capistrano.

* `drush kw-new-project`: Creates a `src` directory in which the source of a the
  new project is placed, based on a provided skeleton Git repository. Although 
  it is easily possible to create a new project from scratch without using this
  command, this will get you up to speed more quickly and allows for a 
  company-wide standard on how Drupal projects are constructed (like modules
  that always should be enabled or desired default configuration).
* `drush kw-setup`: Creates a `cnf` directory in which a few files and a 
  directory are created. The directory is called `files` and is intended to 
  become the directory in which the user uploaded files in Drupal are stored.
  The files are called `environment`, `settings.php` and `settings.local.php`.
  The `environment` file contains the type of enviroment, like 'production' 
  (which is the default), 'staging', 'testing' or 'development'.