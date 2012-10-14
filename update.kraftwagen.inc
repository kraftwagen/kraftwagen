<?php

/**
 * @file
 * This file contains the functions that are required to execute 
 * `drush kraftwagen-update`.
 */

/**
 * Implements drush_COMMAND() for `drush kraftwagen-update`.
 */
function drush_kraftwagen_update() {
  $available_commands = drush_get_commands();

  $commands = kraftwagen_context_get_option('update-commands');
  foreach ($commands as $command => $arguments) {
    if (isset($available_commands[$command])) {
      drush_invoke_process('@self', $command, $arguments);
    }
  }
}
