<?php

/**
 * @file
 * This file contains the functions that are required to execute
 * `drush kw-update-makefile`.
 */
use Drush\UpdateService\ReleaseInfo;
/**
 * Implements drush_COMMAND() for `drush kw-update-makefile`.
 *
 * @param string $location
 *   The location of the make file to be updated.
 */
function drush_kraftwagen_kw_update_makefile($makefile) {
  require_once dirname(__FILE__) . '/includes/kraftwagen.info_files.inc';

  $info = kraftwagen_read_info_file($makefile);
  $update_info = (version_compare(drush_core_version(), '7', '>='))
    ? kraftwagen_update_projects78($info) : kraftwagen_update_projects($info);

  if (empty($update_info)) {
    drush_log(dt('No updates required.'), 'success');
    return;
  }

  if (!kraftwagen_update_makefile($makefile, $update_info)) {
    return FALSE;
  }

  drush_log(dt('Updated !makefile.', array('!makefile' => $makefile)), 'success');
}

/**
 * Reads a makefile for projects and returns the updated versions. For drush 7+.
 *
 * @param array $info
 *   The make file info array
 *
 * @return array
 *   The update info
 */
function kraftwagen_update_projects78($info) {
  $release_info = drush_get_engine('release_info');

  foreach($info['projects'] as $project_name => $project) {
    if (isset($project['download'])) {
      drush_log(dt('Project !project is downloaded explicitly instead of normal contrib or core behavior. We will not try to update this.', array('!project' => $project_name)), 'ok');
      continue;
    }

    // Projects without a version specified are serious upgrade risk.
    if (empty($project) || empty($project['version'])) {
      drush_log(dt('No version specified for project !project. This can lead to unintended upgrades.', array('!project' => $project_name)), 'warning');
      // Don't try to determine a better version.
      continue;
    }

    // Projects with just a major version specified are theoretically a smaller
    // risk, but should still be avoided.
    $exploded_version = explode('.', $project['version']);
    if (count($exploded_version) < 2) {
      drush_log(dt('Only a major version specified for project !project. This can lead to unintended upgrades.', array('!project' => $project_name)), 'warning');
      // Don't try to determine a better version.
      continue;
    }
    $full_projectname = $project_name . '-' . $info['core'] . '-' . $project['version'];

    // Create a new project array that is suitable to pass into
    // release_info_fetch.
    $request = pm_parse_request($full_projectname, drush_get_option('make-update-default-url', ReleaseInfo::DEFAULT_URL));
    if (!($project_data_retrieved = getAppropriateRelease($release_info, $request))) {
      drush_log(dt('Project !project could not be found and was not updated'), array('!project' => $project_name), 'warning');
      continue;
    }

    $version_string = $project_data_retrieved['version_major'] . '.' . $project_data_retrieved['version_patch'];
    if (!empty($project_data_retrieved['version_extra'])) {
      $version_string .= '-' . $project_data_retrieved['version_extra'];
    }
    if (version_compare($version_string, $project['version'], '>')) {
      $update_info[$project_name] = array(
        'current_version' => $info['projects'][$project_name]['version'],
        'new_version' => $version_string
      );
    }
  }

  return $update_info;
}

/**
 * Reads a makefile for projects and returns the updated versions.
 *
 * @param array $info
 *   The make file info array
 *
 * @return array
 *   The update info
 */
function kraftwagen_update_projects($info) {
  drush_include_engine('release_info', 'updatexml');

  $update_info = array();

  foreach($info['projects'] as $project_name => $project) {
    if (isset($project['download'])) {
      drush_log(dt('Project !project is downloaded explicitly instead of normal contrib or core behavior. We will not try to update this.', array('!project' => $project_name)), 'ok');
      continue;
    }

    // Projects without a version specified are serious upgrade risk.
    if (empty($project) || empty($project['version']) || $project['version'] == DRUSH_MAKE_VERSION_BEST) {
      drush_log(dt('No version specified for project !project. This can lead to unintended upgrades.', array('!project' => $project_name)), 'warning');
      // Don't try to determine a better version.
      continue;
    }

    // Projects with just a major version specified are theoretically a smaller
    // risk, but should still be avoided.
    $exploded_version = explode('.', $project['version']);
    if (count($exploded_version) < 2) {
      drush_log(dt('Only a major version specified for project !project. This can lead to unintended upgrades.', array('!project' => $project_name)), 'warning');
      // Don't try to determine a better version.
      continue;
    }

    // Get major version.
    $major = $exploded_version[0];

    // Create a new project array that is suitable to pass into
    // release_info_fetch.
    $request = array(
      'name'     => $project_name,
      'version'  => $info['core'] . '-' . $major,
      'drupal_version' => $info['core'],
      'status url' => drush_get_option('make-update-default-url', RELEASE_INFO_DEFAULT_URL),
    );

    // Try to retrieve project data from drupal.org.
    if (!($project_data_retrieved = release_info_fetch($request)) || $project_data_retrieved === $project_data) {
      // If no data is retrieved, this project is probably not updatable via
      // drupal.org.
      drush_log(dt('Project !project could not be found and was not updated'), array('!project' => $project_name), 'warning');
      continue;
    }

    $version_string = $project_data_retrieved['version_major'] . '.' . $project_data_retrieved['version_patch'];
    if (!empty($project_data_retrieved['version_extra'])) {
      $version_string .= '-' . $project_data_retrieved['version_extra'];
    }
    if (version_compare($version_string, $project['version'], '>')) {
      $update_info[$project_name] = array(
        'current_version' => $info['projects'][$project_name]['version'],
        'new_version' => $version_string
      );
    }
  }

  return $update_info;
}

function kraftwagen_update_makefile($makefile, $update_info) {
  $contents = file_get_contents($makefile);

  foreach ($update_info as $project_name => $info) {
    $new_contents = preg_replace(
      '/(' . preg_quote($project_name) . '[^\n]*\[version\][^\n]*)' . preg_quote($info['current_version']) . '/',
      '${1}' . $info['new_version'],
      $contents
    );

    if ($new_contents == $contents) {
      $new_contents = preg_replace(
        '/(' . preg_quote($project_name) . '[^\n]*[^\n]*)' . preg_quote($info['current_version']) . '/',
        '${1}' . $info['new_version'],
        $contents
      );

      if ($new_contents == $contents) {
        drush_log(dt('Could not update !project_name.', array('!project_name' => $project_name)), 'warning');
      }
    }
    $contents = $new_contents;
  }

  file_put_contents($makefile, $contents);

  return TRUE;
}

/**
 * Obtain the most appropiate release for the requested project.
 *
 * @return array|bool
 *    The selected release xml object or FALSE.
 */
function getAppropriateRelease($release_info, $request) {
  $project_release_info = $release_info->get($request);
  if (!$project_release_info) {
    return FALSE;
  }

  if (drush_get_option('security-only')) {
    return getSecurityRelease($project_release_info);
  }

  // If there was no specific release requested, try to identify the most appropriate release.
  return $project_release_info->getRecommendedOrSupportedRelease();
}

/**
 * Pick the latest security release from XML list.
 *
 * @return array|bool
 *    The selected release xml object or FALSE.
 */
function getSecurityRelease($release_info) {
  $info = $release_info->getInfo();
  $releases = $info['releases'];

  if (!empty($releases)) {
    foreach ($releases as $one_release) {
      if (!empty($one_release['terms']['Release type'])
        && in_array('Security update', $one_release['terms']['Release type'])) {

        return $one_release;
      }
    }
  }

  return FALSE;
}
