<?php

/**
 * Implements hook_drush_command().
 */
function kraftwagen_drush_command() {
  $items = array();

  $items['kraftwagen-create-project'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => dt('Create a new project. This will setup the basic directory and settings stuff for you.'),
    'arguments' => array(
      'directory' => dt('The directory to create the project in. Defaults to the current directory'),
    ),
    'aliases' => array('kw-cp'),
    'options' => array(
      'force' => dt('Forces execution of this command, even if the specified source dir already exists.'),
    ),
  );

  $items['kraftwagen-build'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => dt('Create a build from the source'),
    'aliases' => array('kw-b'),
  );

  $items['kraftwagen-make-update'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'description' => dt('Update an existing makefile to the newest version of contributed modules. This will NOT do any major updates.'),
    'arguments' => array(
      'makefile.make' => dt('The makefile to update. As this command will actually change this file, it should be readable and writeable, and you should backup it, or have it under version control.')
    ),
    'required-arguments' => 1,
    'aliases' => array('kw-mu'),
  );

  $items['kraftwagen-dependencies'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
    'description' => 'Check if all required modules are enabled, enable them if they are not, and disable them if the are enabled, but not required.',
    'aliases' => array('kw-d'),
  );

  return $items;
}

/**
 * Implements hook_drush_init().
 */
function kraftwagen_drush_init() {
  // load the context handling and fill it with the default values in
  // kraftwagenrc.php.
  require_once dirname(__FILE__) . '/kraftwagen.context.inc';
  kraftwagen_context_load_default_config();
}
