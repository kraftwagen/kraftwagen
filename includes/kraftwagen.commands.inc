<?php

/**
 * Run a sequence of drush commands that are defined in Kraftwagen context 
 * option.
 */
function kraftwagen_commands_sequence_run($option, $replacements = array(), $fail_on_error = TRUE) {
  $commands = kraftwagen_context_get_option($option);
  $commands = kraftwagen_commands_sequence_replace($commands, $replacements);
  kraftwagen_commands_sequence_run_commands($commands, $fail_on_error);
}

/**
 * Run a sequence of drush commands.
 */
function kraftwagen_commands_sequence_run_commands($commands, $fail_on_error = TRUE) {
  foreach ($commands as $command => $arguments) {
    if (is_numeric($command)) {
      if (is_array($arguments)) {
        kraftwagen_commands_sequence_run_commands($arguments);
        continue;
      } else {
        $command = $arguments;
        $arguments = array();
      }
    }

    $phase_index = drush_get_context('DRUSH_BOOTSTRAP_PHASE');
    if ($phase_index >= DRUSH_BOOTSTRAP_DRUPAL_FULL) {
      _drush_find_commandfiles(DRUSH_BOOTSTRAP_DRUPAL_FULL);
    }
    $available_commands = drush_get_commands();
    
    if (isset($available_commands[$command])) {
      $result = drush_invoke_process('@self', $command, $arguments);
      if (!$result || !empty($result['error_status'])) {
        $return = drush_set_error(dt("Called command !command returned an error.", array('!command' => $command)));
        if ($fail_on_error) {
          return $return;
        }
      }
    } else {
      $return = drush_set_error(dt("Command !command not found", array('!command' => $command)));
      if ($fail_on_error) {
        return $return;
      }
    }
  }
}

function kraftwagen_commands_sequence_replace($data, $replacements) {
  if (is_array($data)) {
    $result = array();
    foreach ($data as $key => $value) {
      $key = kraftwagen_commands_sequence_replace($key, $replacements);
      $value = kraftwagen_commands_sequence_replace($value, $replacements);
      $result[$key] = $value;
    }
    return $result;
  } elseif (is_string($data)) {
    return str_replace(array_keys($replacements), array_values($replacements), $data);
  } else {
    return $data;
  }
}