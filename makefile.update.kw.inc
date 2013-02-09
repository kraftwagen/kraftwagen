<?php

/**
 * @file
 * This file contains the functions that are required to execute 
 * `drush kw-update-makefile`.
 */

/**
 * Implements drush_COMMAND() for `drush kw-update-makefile`.
 */
function drush_kraftwagen_kw_update_makefile($makefile) {
  require_once dirname(__FILE__) . '/includes/kraftwagen.info_files.inc';

  // parse
  $info = kraftwagen_parse_info_file($makefile);
  if ($info === FALSE) {
    return drush_set_error(dt('Could not open make file !makefile', array('!makefile' => $makefile)));
  }
  if (($info = make_validate_info_file($info)) === FALSE) {
    return drush_set_error(dt('Could not validate make file !makefile', array('!makefile' => $makefile)));
  }
  
  // get updated version of the info
  $info_updated = kraftwagen_update_projects($info);
  
  if ($info === $info_updated) {
    drush_log(dt('There are no changes to the makefile, so we did not write a new version.'), 'ok');
    return;
  }
  
  // generate makefile from info
  $data = kraftwagen_generate_info_file($info_updated);
  
  // write data to file
  file_put_contents($makefile, $data);
  
  drush_log(dt('The changes have been written to the makefile.'), 'ok');
}

/**
 * Reads a makefile for projects and updates the versions.
 * 
 * @param $info
 *   The make file info array
 * 
 * @return
 *   The updated info array
 */
function kraftwagen_update_projects($info) {
  drush_include_engine('release_info', 'updatexml');

  foreach($info['projects'] as $project_name => $project) {
    if (isset($project['download'])) {
      drush_log(dt('Project !project is downloaded explicitly instead of normal contrib or core behavior. We will not try to update this.', array('!project' => $project_name)), 'success');
    }

    // projects without a version specified are serious upgrade risk
    if (empty($project) || empty($project['version']) || $project['version'] == DRUSH_MAKE_VERSION_BEST) {
      drush_log(dt('No version specified for project !project. This can lead to unintended upgrades.', array('!project' => $project_name)), 'warning');
      continue; // don't try to determine a better version
    }
    
    // projects with just a major version specified are theoretically a smaller
    // risk, but should still be avoided
    $exploded_version = explode('.', $project['version']);
    if (count($exploded_version) < 2) {
      drush_log(dt('Only a major version specified for project !project. This can lead to unintended upgrades.', array('!project' => $project_name)), 'warning');
      continue; // don't try to determine a better version
    }
    
    // get major version
    $major = $exploded_version[0];
    
    // create a new project array that is suitable to pass into 
    // release_info_fetch
    $request = array(
      'name'     => $project_name,
      'version'  => $info['core'] . '-' . $major,
      'drupal_version' => $info['core'],
      'status url' => drush_get_option('make-update-default-url', RELEASE_INFO_DEFAULT_URL),
    );
    
    // try to retrieve project data from drupal.org
    if (!($project_data_retrieved = release_info_fetch($request)) || $project_data_retrieved === $project_data) {
      // if no data is retrieved, this project is probably not updatable via 
      // drupal.org
      drush_log(dt('Project !project could not be found and was not updated'), array('!project' => $project_name), 'warning');
      continue;
    }
    
    // only update to the new version if the suggested version is actually newer than our currently specified version
    if (version_compare($project_data_retrieved['version'], $project['version'], '>')) {
      $info['projects'][$project_name]['version'] = $project_data_retrieved['version'];
      drush_log(dt('Project !project has been upgraded to version !version.', array('!project' => $project_name, '!version' => $project_data_retrieved['version'])), 'success');
    } else {
      drush_log(dt('Project !project has not been upgraded because it already is at the latest version.', array('!project' => $project_name, '!version' => $project['version'])), 'ok');
    }
  }
  
  return $info;
}
