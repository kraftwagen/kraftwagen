<?php

/**
 * @file
 * This file contains the functions that are required to execute
 * `drush kw-new-project`.
 */

/**
 * Implements drush_COMMAND for `drush kw-new-project`.
 * 
 * @param string $dir
 *   Directory where to create the new project. Optional. Defaults to the 
 *   current working directory.
 */
function drush_kraftwagen_kw_new_project($argdir = NULL) {
  if (!file_exists(realpath(getcwd() . '/' . $argdir))){
    drush_mkdir(getcwd() . '/' . $argdir);
  }
  $dir = realpath($argdir ? $argdir : getcwd());
  if (!$dir) {
    drush_log(dt('The directory !dir could not be created.', array('!dir' => './' . $argdir)), 'error');
    return;
  }

  // If the dir has a kraftwagenrc file, we should load it.
  if (file_exists($dir . DIRECTORY_SEPARATOR . KRAFTWAGEN_RC)) {
    kraftwagen_context_load_config_file($dir . DIRECTORY_SEPARATOR . KRAFTWAGEN_RC);
  }

  // Determine source directory.
  $dir_src = kraftwagen_context_get_option('src-dir');
  if (is_dir($dir . DIRECTORY_SEPARATOR . $dir_src) && !drush_get_option('force')) {
    drush_log(dt('The directory !dir already exists. Use --force if you really want Kraftwagen to overwrite files in this directory.', array('!dir' => $dir_src)), 'error');
    return;
  }

  // Ask for the human readable name of the project. This can be used for the
  // name property of the install profile. Defaults to the name of the current
  // directory.
  $project_label = drush_get_option(array('human-name', 'hn'));
  if (!$project_label) {
    $project_label = drush_prompt(dt('Enter the human readable name of the project'), basename($dir));
  }

  // Create machine name for the project. This defaults to a simplified version
  // of the human readable name. This is used for the file names.
  $project_name = drush_get_option(array('name', 'n'));
  if (!$project_name) {
    $project_name = _kraftwagen_drush_prompt_validate(
      dt('Enter the internal machine name of the project'),
      _kraftwagen_generate_name($project_label), TRUE, '_kraftwagen_validate_name'
    );
  }

  // Ask for the location of the git repository that contains the skeleton.
  $skeleton = drush_get_option(array('skeleton'));
  $branch = drush_get_option(array('branch'));
  if (!$skeleton) {
    $skeleton = drush_prompt(dt('Enter the location of the skeleton repository'), 'git://github.com/kraftwagen/skeleton.git');
  }
  if (!$branch) {
    $branch = drush_prompt(dt('Enter the name of the branch to use [source repository HEAD]'), NULL, FALSE);
  }

  // Create a tempdir where we are going to clone the repository in.
  $tempdir = drush_tempdir();

  // Clone the skeleton repository.
  if ($branch) {
    $args = array('git clone -b %s %s %s', $branch, $skeleton, $tempdir);
  }
  else {
    $args = array('git clone %s %s', $skeleton, $tempdir);
  }
  if (call_user_func_array('drush_shell_exec', $args)) {
    // Remove the repository nature of the thing.
    if (file_exists($tempdir . '/.git')) {
      drush_shell_exec("rm -rf %s", $tempdir . '/.git');
    }

    // Make sure we have a src directory. This is going to contain the whole
    // actual source of the project.
    drush_mkdir($dir . DIRECTORY_SEPARATOR . $dir_src);

    _kraftwagen_copy_dir($tempdir, $dir . DIRECTORY_SEPARATOR . $dir_src,
      array('SKELETON' => $project_name),
      array(
        '***MACHINE_NAME***' => $project_name,
        '***HUMAN_NAME***' => $project_label,
        '***HUMAN_NAME_ESCAPED***' => addslashes($project_label)
      )
    );

    drush_log(dt('Copied the project skeleton to your source directory.'), 'ok');
  }
  else {
    return drush_set_error(dt('Could not download the repository from %path', array('%path' => $skeleton)));
  }
}

/**
 * Internal helper function. Create a system name from a human readable name.
 *
 * @param string $label
 *   A human readable project name
 *
 * @return string
 *   The project name converted to a allowed system name
 */
function _kraftwagen_generate_name($label) {
  // Convert to lowercase, replace every non-alphanumeric character with an
  // underscore, replace all repeating underscores with a single underscore,
  // make sure that the name does not start with a number.
  return preg_replace('/^([0-9])/','_\1', preg_replace('/_+/','_', preg_replace('/[^a-z0-9]/', '_', strtolower($label))));
}

/**
 * Interal helper function. Check if a given name is a valid system name.
 *
 * @param string $name
 *   The name to validate
 *
 * @return
 *   TRUE if the name is valid, FALSE if it isn't
 */
function _kraftwagen_validate_name($name) {
  // Make sure it consists only of alphanumeric characters and underscores and
  // does not start with a number.
  if (!preg_match('/^[a-z_][a-z0-9_]*$/', $name)) {
    return drush_set_error(dt('Only lowercase alphanumeric characters and underscores are allowed. The input cannot start with a number.'));
  }

  return TRUE;
}

/**
 * Internal helper function. Wrapper around drush_prompt that runs the input
 * through a validate callback.
 *
 * @param string $prompt
 *   The text which is displayed to the user.
 * @param string $default
 *   Optional. The default value of the input.
 * @param bool $required
 *   Optional. If TRUE, user may continue even when no value is in the input.
 * @param string $validate_callback
 *   Optional. The callback to call to check if the input is valid.
 *
 * @return string
 *   The user input, after being validated.
 */
function _kraftwagen_drush_prompt_validate($prompt, $default = NULL, $required = TRUE, $validate_callback = NULL) {
  while (TRUE) {
    $result = drush_prompt($prompt, $default, $required);

    // Assume that the default is a valid result.
    if ($result == $default) {
      return $result;
    }

    // If we have a callback and the callback returns true.
    if ($validate_callback && $validate_callback($result)) {
      return $result;
    }
  }
}

/**
 * Internal helper function. Recursively copy a directory, while replacing
 * strings in file and directory names and in file contents.
 *
 * @param string $src_dir
 *   The path to the directory to copy from
 * @param string $dest_dir
 *   The path of the directory to copy to
 * @param array $filename_replacements
 *   Optional. Associative array of string replacements in file and directory
 *   names.
 * @param array $content_replacements
 *   Optional. Associative array of string replacements in file contents.
 */
function _kraftwagen_copy_dir($src_dir, $dest_dir, $filename_replacements = array(), $content_replacements = array()) {
  // Find the files in the src_dir. The grep and sed, remove that prefixing dot.
  drush_shell_cd_and_exec($src_dir, 'find . -mindepth 1 | grep "^\." | sed -e "s/^\.//"');
  $files = drush_shell_exec_output();

  // Make sure the kraftwagen_file_replace function is available.
  require_once dirname(__FILE__) . '/includes/kraftwagen.fileutils.inc';

  // Loop over the files. $file starts with a slash.
  foreach ($files as $file) {
    // Create the full path of the file in the src_dir.
    $src_path = $src_dir . $file;
    // Create the full path that the file should get in the working directory.
    $dest_path = $dest_dir . (!empty($filename_replacements) ? str_replace(array_keys($filename_replacements), array_values($filename_replacements), $file) : $file);

    if (is_dir($src_path)) {
      drush_mkdir($dest_path);
    }
    else {
      kraftwagen_file_replace($src_path, $dest_path, $content_replacements);
    }
  }
}
