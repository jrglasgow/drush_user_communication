<?php

/**
 * @file
 *
 * Adds a drush command to see what users are currently active on the site.
 */


 /**
 * Implements hook_drush_command().
 */
function user_communication_drush_command() {
  $items = array();

  $items['wall'] = array(
    'description'   =>  'Send a message to all active user on the site which they will see on their next page load.',
    'arguments'     => array(
      'message'  => 'The message to send to ths users.',
      'type'    => 'The type of message: status, warning, error. defaults to status.',
    ),
    'bootstrap'     => DRUSH_BOOTSTRAP_DRUPAL_FULL, // Full bootstrap into Drupal,
  );
  $items['write'] = array(
    'description'   =>  'Send a message to all active user on the site which they will see on their next page load.',
    'arguments'     => array(
      'user'      => 'The name of the user to send the message to.',
      'message'   => 'The message to send to ths users.',
      'type'    => 'The type of message: status, warning, error. defaults to status.',
    ),
    'bootstrap'     => DRUSH_BOOTSTRAP_DRUPAL_FULL, // Full bootstrap into Drupal,
  );

  return $items;
}

/**
 * send a message to all users
 */
function drush_user_communication_wall($message = '', $type = 'status') {
  // get a list of active users/sessions
  $result = db_query("SELECT * FROM {sessions} s JOIN {users} u ON u.uid=s.uid WHERE s.uid > 0")->fetchAll();

  if (empty($result)) {
    drush_print(dt('There are no users currently logged in.'));
  }
  else {
    foreach ($result AS $session_row) {
      if (user_communication_write($session_row->uid, $message, $type)) {
        $users[] = $session_row->name;
      }
    }
  }
  if (!empty($users)) {
    drush_print('');
    drush_print(dt('The message has been sent to the following users:'));
    foreach ($users AS $user) {
      drush_print($user);
    }
  }
}

/**
 * Write a message to a single user.
 */
function drush_user_communication_write($user_name, $message, $type = 'status') {
  $user = db_query("SELECT * FROM {users} u WHERE u.name=:name", array(':name' => $user_name))->fetch();
  if (user_communication_write($user->uid, $message, $type)) {
    drush_print('');
    drush_print(dt('The message has been sent to the following users:'));
    drush_print($user_name);
  }
}

/**
 * set a message for the user to read
 */
function user_communication_write($uid, $message, $type) {
  $session = db_query("SELECT * FROM {sessions} s WHERE s.uid=:uid", array(':uid' => $uid))->fetch();

  $key = array();
  if ($session->sid) {
    $key['sid'] = $session->sid;
  }
  else if ($session->ssid) {
    $key['ssid'] = $session->ssid;
  }
  $session = unserialize_session($session->session);
  if (empty($session)) {
    $session = array();
  }

  $session['messages'][$type][] = dt('Message from site administration - !time:', array('!time' => format_date(REQUEST_TIME, 'custom', 'd M Y H:i:s T'))) . '<br/>' . $message;
  return db_merge('sessions')
    ->key($key)
    ->fields(array('session' => serialize_session($session)))
    ->execute();
}

/**
 * Unserialize the session data originally encoded by session_encode().
 */
function unserialize_session($data) {
  $vars=preg_split('/([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff^|]*)\|/',
            $data,-1,PREG_SPLIT_NO_EMPTY | PREG_SPLIT_DELIM_CAPTURE);
  for($i=0; $vars[$i]; $i++) $result[$vars[$i++]]=unserialize($vars[$i]);
  return $result;
}

/**
 * Serialize the session data.
 */
function serialize_session($session = array()) {
  $text = '';
  if (!empty($session)) {
    foreach ($session AS $key => $value) {
      $text .= $key . '|' . serialize($value);
    }
  }
  return $text;
}
