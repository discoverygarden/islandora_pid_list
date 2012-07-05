<?php

/**
 * @file
 * Creates the forms used in islandora_pid_list
 * 
 */

/**
 * Admin settings to allow the enabling and disabling of the 'Lists' tab
 */
function islandora_pid_list_settings() {
  $form = array();
  $form['islandora_pid_list_show_tabs'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show Pid List Tabs'),
    '#description' => t("Show a tab for the PID lists associated to each object on each Fedora object's page."),
    '#default_value' => variable_get('islandora_pid_list_show_tabs', TRUE),
  );
  return system_settings_form($form);
}

/**
 * Form constructor for the pidlist list form.
 * 
 * @global type $user
 * 
 * @param array $form_state
 *   The Drupal form state.
 * 
 * @see islandora_pid_list_menu()
 */
function islandora_pid_list_list_form($form_state) {
  drupal_add_css(drupal_get_path('module', 'islandora_pid_list') . '/css/islandora_pid_list.css');
  module_load_include('inc', 'php_lib', 'Ahah');
  module_load_include('inc', 'islandora_pid_list', 'islandora_pid_list.api');
  module_load_include('inc', 'islandora_pid_list', 'pidlist');
  module_load_include('inc', 'fedora_repository', 'api/tuque');
  global $user;
    
  if (isset($_GET['collapsed']) && $_GET['collapsed'] == $value->listid) {
    $form['param'][$key]['#collapsed'] = FALSE;
    unset($_GET['collapsed']);
  }
  
  Ahah::get_ahah_js();
  
  // Get all users for use in select for forms
  $query = db_query("SELECT uid, name FROM {users} ORDER BY uid", $user->uid);
  
  while ($result = db_fetch_object($query)) {
    $resultname = $result->name;
      if ($result->uid == 0) {
        $resultname = 'Anonymous';
      }
    $options[$result->uid] = array(
      'uid' => $result->uid,
      'name' => $resultname
    );
  }
  
  $pidlist = get_user_pid_list(array('user' => $user->uid));
  
  $form = array(
    '#tree' => TRUE,
    '#prefix' => '<div id="allpidlists">',
    '#suffix' => '</div>',
  );
  $form['userlists'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#title' => t('My Lists'),
  );
  
  if (!empty($pidlist)) {
    foreach ($pidlist as $key => $value) {
      $idoutput = str_replace('session_', '', $value->listid);
      $form['userlists'][$key] = array( 
        '#type' => 'fieldset',
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#title' => check_plain($value->listname . ' [ID:' . $idoutput . ']'),
      );
      
      if (isset($form_state['storage']['listid']) && $value->listid == $form_state['storage']['listid']) {
        $form['userlists'][$key]['#collapsed'] = FALSE;
        unset($form_state['storage']['listid']);
      }
      
      if (isset($_GET['collapsed']) && $_GET['collapsed'] == $value->listid) {
        $form['userlists'][$key]['#collapsed'] = FALSE;
      }
      
      if (user_access("Manage other users' pidlists") || $user->uid === $value->listowner) {
        $form['userlists'][$key]['listname'] = array(
          '#type' => 'textfield',
          '#title' => t('List Name'),
          '#size' => 20,
          '#value' => $value->listname,
          '#prefix' => '<div class="listdetails">',
          '#ahah' => array(
            'event' => '',
            'keypress' => TRUE,
            'path' => "pidlist/ahah/updatename",
            'wrapper' => "allpidlists",
            'method' => 'replace',          
          ),
        );

        $form['userlists'][$key]['changename'] = array(
          '#type' => 'image_button',
          '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/add.png', 
          '#suffix' => '</div>',
          '#ahah' => array(
              'event' => 'click',
              'path' => "pidlist/ahah/updatename",
              'wrapper' => "allpidlists",
              'method' => 'replace',
              ),
        );  

        $form['userlists'][$key]['labelremove'] = array(
          '#type' => 'item',
          '#title' => t('Delete this list'),
          '#weight' => 20000,
          '#prefix' => '<div class="removelist">',
        );

        $form['userlists'][$key]['remove'] = array(
          '#type' => 'image_button',
          '#weight' => 20001,
          '#src' => drupal_get_path('module', 'fedora_repository') . '/images/purge.gif', 
          '#suffix' => '</div>',
          '#ahah' => array(
              'event' => 'click',
              'path' => "pidlist/ahah/remove/list",
              'wrapper' => "allpidlists",
              'method' => 'replace',
              ),
        );
      }

      $form['userlists'][$key]['list-id'] = array(
          '#type' => 'value',
          '#value' => $key,
      );  
      $form['userlists'][$key]['plo'] = array(
        '#type' => 'value',
        '#value' => $value,
      );

    $fku =& $form['userlists'][$key]['users'];

    if (user_access("Manage other users' pidlists") || $user->uid === $value->listowner) {
      $fku = array( 
        '#type' => 'fieldset',
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
        '#title' => t('Users'),
      );

      if (count($value->userlist) == 0) {
          $fku[$itr]['empty'] = array(
            '#type' => 'item',
            '#value' => t('There are no users associated with this list'),
          );
      }
      else {
        foreach ($value->userlist as $itr => $piduser) {
          $tempuser = user_load($piduser);
            if ($tempuser->uid == 0) {
              $tempuser = t('Anonymous');
            }
            elseif ($tempuser->uid == $user->uid) {
              $tempuser = t('Yourself');
            }
            else {
              $tempuser = $tempuser->name;
            }

            $fku[$itr] = array(
              '#prefix' => '<div class="users">',
              '#suffix' => '</div>',
              'label' => array(
                '#type' => 'item',
                '#value' => $tempuser,
              ),
              'user' => array(
                '#type' => 'value',
                '#value' => $piduser,
              ),
              'list-id' => array(
                '#type' => 'value',
                '#value' => $key,
              ),
              'remove' => array(
                '#type' => 'image_button',
                '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/minus_small.png',
                '#ahah' => array(
                  'event' => 'click',
                  'path' => "pidlist/ahah/remove/user",
                  'wrapper' => "allpidlists",
                  'method' => 'replace',
                ),
              ),
              'plo' => array(
                '#type' => 'value',
                '#value' => $value,
              )
            ); 
          }   
        }
      
      $selectoptions = array("" => "- Select A User To Add - ");
      
      foreach ($options as $index => $item) {
        if (!in_array($index, $value->userlist)) {
        $selectoptions[$index] = $item['uid'] . ': ' . $item['name'];
        }  
      }
      $fku['listusers'] = array(
        '#type' => 'select',
        '#default value' => t('Select a user to add'),
        '#title' => t('Add a user to the list'),
        '#options' => $selectoptions,
        '#width' => 20,
        '#weight' => 10000,
        '#prefix' => '<div class="addusertolist">',
      );
       $fku['listadd'] = array(
        '#type' => 'image_button',
        '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/add.png', 
        '#suffix' => '</div>',
        '#weight' => 10001, 
        '#ahah' => array(
          'event' => 'click',
          'path' => "pidlist/ahah/add/user",
          'wrapper' => "allpidlists",
          'method' => 'replace',
        ),
      ); 
    }
    elseif (in_array($user->uid, $value->userlist)) {
      $piduser = $user->uid;
      $tempuser = user_load($piduser);
      $fku = array( 
        '#type' => 'fieldset',
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
        '#title' => t('Users'),
      );

      $tempuser = t('Yourself');
      
      $fku[] = array(
        '#prefix' => '<div class="users">',
        '#suffix' => '</div>',
        'label' => array(
          '#type' => 'item',
          '#value' => $tempuser,
        ),
        'user' => array(
          '#type' => 'value',
          '#value' => $piduser,
        ),
        'list-id' => array(
          '#type' => 'value',
          '#value' => $key,
        ),
        'remove' => array(
          '#type' => 'image_button',
          '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/minus_small.png',
          '#ahah' => array(
            'event' => 'click',
            'path' => "pidlist/ahah/remove/user",
            'wrapper' => "allpidlists",
            'method' => 'replace',
          ),
        ),
        'plo' => array(
          '#type' => 'value',
          '#value' => $value,
        )
      );      
    } 
    $form['userlists'][$key]['pids'] = array( 
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#title' => t('Pids'),
    );
    $fkp =& $form['userlists'][$key]['pids']; 
    
    if (!empty($value->pidlist)) {
      foreach ($value->pidlist as $itr => $pid) {
        $fedoraobject = islandora_object_load($pid);
        $pidvalue = $pid;

        if ($fedoraobject) {
          $pidvalue = l($pid, 'fedora/repository/' . $pid, array(
            'attributes' => array(
              'target' => '_blank'
              )));
        }

        $fkp[$itr] = array(
          '#prefix' => '<div class="pids">',
          '#suffix' => '</div>',
          'label' => array(
            '#type' => 'item',
            '#value' => $pidvalue,
          ),
          'pid' => array(
            '#type' => 'value',
            '#value' => $pid,
          ),
          'plo' => array(
              '#type' => 'value',
              '#value' => $value,
          ),
          'list-id' => array(
              '#type' => 'value',
              '#value' => $key,
          ),
        );
        if (user_access("Manage other users' pidlists") || $user->uid === $value->listowner) {
          $fkp[$itr]['remove'] = array(
            '#type' => 'image_button',
            '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/minus_small.png',
            '#ahah' => array(
                'event' => 'click',
                'path' => "pidlist/ahah/remove/pid",
                'wrapper' => "allpidlists",
                'method' => 'replace',
                ),
          );
        }
      }
    }
    else {
      $fkp['empty'] = array(
        '#prefix' => '<div class="pids">',
        '#suffix' => '</div>',
        '#value' => t('No pids currently associated'),
      );
    }
     if (user_access("Manage other users' pidlists") || $user->uid === $value->listowner) { 
      $fkp['pidtext'] = array(
        '#type' => 'textfield',
        '#title' => t('Add a PID'),
        '#size' => 20,
        '#prefix' => '<div class="addpid">',
        '#suffix' => '</div>',
        '#weight' => 10000,
        '#ahah' => array(
          'event' => '',
          'keypress' => TRUE,
          'path' => "pidlist/ahah/add/pid",
          'wrapper' => "allpidlists",
          'method' => 'replace',          
        ),
      );
      
      $fkp['add_button'] = array(
        '#type' => 'image_button',
        '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/add.png', 
        '#weight' => 10001,  
        '#ahah' => array(
          'event' => 'click',
          'path' => "pidlist/ahah/add/pid",
          'wrapper' => "allpidlists",
          'method' => 'replace',
        ),
      );
    } 
  }
 }
 else {
   $form['userlists']['empty'] = array(
     '#type' => 'item',
     '#value' => t('No lists currently associated'),
     '#prefix' => '<div class="emptylist">',
     '#suffix' => '</div>',
   );
 }
     
 if (user_access('Adminisiter islandora_pid_list module')) {
    $templists = get_user_pid_list(array('all' => TRUE));
    
    $otherlists = array();
    foreach ($templists as $item) {
      if (!in_array($user->uid, $item->userlist)) {
        $otherlists[] = $item;
      }
    }
    $form['alllists'] = array(
      '#type' => 'fieldset',
      '#title' => t('All Other Lists'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );
    if (!empty($otherlists)) {
      foreach ($otherlists as $key => $value) {
        $form['alllists'][$key] = array( 
          '#type' => 'fieldset',
          '#collapsible' => TRUE,
          '#collapsed' => TRUE,
          '#title' => check_plain($value->listname . ' [ID:' . $value->listid . ']'),  
        );
        
        if (isset($form_state['storage']['listid']) && $value->listid == $form_state['storage']['listid']) {
        $form['alllists'][$key]['#collapsed'] = FALSE;
        unset($form_state['storage']['listid']);
      }
        
        $form['alllists'][$key]['listname'] = array(
          '#type' => 'textfield',
          '#title' => t('List Name'),
          '#size' => 20,
          '#value' => $value->listname,
          '#prefix' => '<div class="listdetails">',
          '#ahah' => array(
              'event' => '',
              'keypress' => TRUE,
              'path' => "pidlist/ahah/updatename",
              'wrapper' => "allpidlists",
              'method' => 'replace',          
          ),
        );

        $form['alllists'][$key]['changename'] = array(
          '#type' => 'image_button',
          '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/add.png', 
          '#suffix' => '</div>',
          '#ahah' => array(
              'event' => 'click',
              'path' => "pidlist/ahah/updatename",
              'wrapper' => "allpidlists",
              'method' => 'replace',
              ),
        );
        
        $form['alllists'][$key]['labelremove'] = array(
          '#type' => 'item',
          '#title' => t('Delete this list'),
          '#weight' => 20000,
          '#prefix' => '<div class="removelist">',
        );
          
        $form['alllists'][$key]['remove'] = array(
          '#type' => 'image_button',
          '#weight' => 20001,
          '#src' => drupal_get_path('module', 'fedora_repository') . '/images/purge.gif', 
          '#suffix' => '</div>',
          '#ahah' => array(
              'event' => 'click',
              'path' => "pidlist/ahah/remove/list",
              'wrapper' => "allpidlists",
              'method' => 'replace',
              ),
        );

        $form['alllists'][$key]['list-id'] = array(
          '#type' => 'value',
          '#value' => $key,
        );  
        $form['alllists'][$key]['plo'] = array(
          '#type' => 'value',
          '#value' => $value,
        );

        $fku =& $form['alllists'][$key]['users'];

        if (user_access("Manage other users' pidlists") || $user->uid === $value->listowner) {
          $fku = array( 
            '#type' => 'fieldset',
            '#collapsible' => TRUE,
            '#collapsed' => FALSE,
            '#title' => t('Users'),
          );

          if (count($value->userlist) == 0) {
              $fku[$itr]['empty'] = array(
                '#type' => 'item',
                '#value' => t('There are no users associated with this list'),
              );
          }
          else {
            foreach ($value->userlist as $itr => $piduser) {
              $tempuser = user_load($piduser);
                if ($tempuser->uid == 0) {
                  $tempuser = t('Anonymous');
                }
                else {
                  $tempuser = $tempuser->name;
                }

                $fku[$itr] = array(
                  '#prefix' => '<div class="users">',
                  '#suffix' => '</div>',
                  'label' => array(
                    '#type' => 'item',
                    '#value' => $tempuser,
                  ),
                  'user' => array(
                    '#type' => 'value',
                    '#value' => $piduser,
                  ),
                  'list-id' => array(
                    '#type' => 'value',
                    '#value' => $key,
                  ),
                  'remove' => array(
                    '#type' => 'image_button',
                    '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/minus_small.png',
                    '#ahah' => array(
                      'event' => 'click',
                      'path' => "pidlist/ahah/remove/user",
                      'wrapper' => "allpidlists",
                      'method' => 'replace',
                    ),
                  ),
                  'plo' => array(
                    '#type' => 'value',
                    '#value' => $value,
                  )
                ); 
            }   
          }
        
        $selectoptions = array("" => "- Select A User To Add - ");
        
        foreach ($options as $index => $item) {
          if ($index == 0) {
            foreach ($value->userlist as $val) {
              if (is_null($val)) {
                $selectoptions[$index] = $item['uid'] . ': ' . $item['name'];
              }
            }
          }
          if (!in_array($index, $value->userlist)) {
            $selectoptions[$index] = $item['uid'] . ': ' . $item['name'];
          }  
        }
        
        $fku['listusers'] = array(
          '#type' => 'select',
          '#default value' => t('Select a user to add'),
          '#title' => t('Add a user to the list'),
          '#options' => $selectoptions,
          '#width' => 20,
          '#weight' => 10000,
          '#prefix' => '<div class="addusertolist">',
        );
        $fku['listadd'] = array(
        '#type' => 'image_button',
        '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/add.png', 
        '#suffix' => '</div>',
        '#weight' => 10001, 
        '#ahah' => array(
          'event' => 'click',
          'path' => "pidlist/ahah/add/user",
          'wrapper' => "allpidlists",
          'method' => 'replace',
        ),
      ); 
              
      $form['alllists'][$key]['pids'] = array( 
        '#type' => 'fieldset',
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
        '#title' => t('Pids'),
      );
      $fkp =& $form['alllists'][$key]['pids']; 

      if (!empty($value->pidlist)) {
        foreach ($value->pidlist as $itr => $pid) {
          $fedoraobject = islandora_object_load($pid);
          $pidvalue = $pid;

          if ($fedoraobject) {
            $pidvalue = l($pid, 'fedora/repository/' . $pid, array(
              'attributes' => array(
                'target' => '_blank'
                )));
          }

          $fkp[$itr] = array(
            '#prefix' => '<div class="pids">',
            '#suffix' => '</div>',
            'label' => array(
              '#type' => 'item',
              '#value' => $pidvalue,
            ),
            'pid' => array(
              '#type' => 'value',
              '#value' => $pid,
            ),
            'list-id' => array(
              '#type' => 'value',
              '#value' => $key,
            ),
            'plo' => array(
              '#type' => 'value',
              '#value' => $value,
            ),
            'remove' => array(
              '#type' => 'image_button',
              '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/minus_small.png',
              '#ahah' => array(
                'event' => 'click',
                'path' => "pidlist/ahah/remove/pid",
                'wrapper' => "allpidlists",
                'method' => 'replace',
              ),
            ),
          );
        }
       }
       else {
          $fkp['empty'] = array(
            '#prefix' => '<div class="pids">',
            '#suffix' => '</div>',
            '#value' => t('No pids currently associated'),
          );
       }
      $fkp['pidtext'] = array(
        '#type' => 'textfield',
        '#title' => t('Add a PID'),
        '#size' => 20,
        '#prefix' => '<div class="addpid">',
        '#suffix' => '</div>',
        '#weight' => 10000,
        '#ahah' => array(
          'event' => '',
          'keypress' => TRUE,
          'path' => "pidlist/ahah/add/pid",
          'wrapper' => "allpidlists",
          'method' => 'replace',          
        ),
      );
      $fkp['add_button'] = array(
        '#type' => 'image_button',
        '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/add.png', 
        '#weight' => 10001,  
        '#ahah' => array(
          'event' => 'click',
          'path' => "pidlist/ahah/add/pid",
          'wrapper' => 'allpidlists',
          'method' => 'replace',
        ),
      );
     }
    }
   }
   else {
    $form['alllists']['empty'] = array(
      '#type' => 'item',
      '#value' => t('No lists currently associated'),
      '#prefix' => '<div class="emptylist">',
      '#suffix' => '</div>',
    );  
   }
 }
  return $form;
}

/**
 * Form constructor for the add pidlist form.
 * 
 * @global type $user
 * @param type $form_state
 * @see islandora_pid_list_add_form_submit
 * @see islandora_pid_list_menu()
 */
function islandora_pid_list_add_form($form_state) {
  global $user;
  
  $options = array("" => "- Select A User To Add - ");
  $query = db_query("SELECT uid, name FROM {users} WHERE uid <> %d ORDER BY uid", $user->uid);
  while ($result = db_fetch_object($query)) {
    $resultname = $result->name;
      if ($result->uid == 0) {
        $resultname = 'Anonymous';
      }
    $options[$result->uid]= $result->uid . ': ' . $resultname;
  }
    
  $form = array(
    '#title' => t('Add'),
    '#tree' => TRUE,
    'listname' => array(
      '#type' => 'textfield',
      '#title' => t('List Name'),
      '#size' => 20, 
    ),
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#name' => 'islandora_pid_list_add_form_submit_button',
    '#value' => t('Submit'),
  );
  $form['clear'] = array(
    '#type' => 'submit',
    '#name' => 'islandora_pid_list_add_form_clear_button',
    '#value' => t('Clear'),
  );
  
  return $form; 
}

/**
 * Form submission handler for the islandora_pid_list_add_form().
 * 
 * @global type $user
 * @param array $form
 * @param array $form_state 
 * @see islandora_pid_list_add_form()
 */
function islandora_pid_list_add_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_pid_list', 'pidlist');
    
  global $user;
  
  $createname = $form_state['values']['listname'];
 
  // Create a new list
  if (!ctype_space($createname) && $createname != '') {
    $paramusers = array();
    $parampids = array();

    $params = array(
      'retrieve' => FALSE,
      'useruid' => $user->uid,
      'listowner' => $user->uid,
      'listname' => $createname,
      'listusers' => array($user->uid),
    );

    if ($user->uid == 0) {
      $params['session'] = TRUE;
    }

    $templist = new PidList($params);

    if ($user->uid == 0) {
      $savedlist = 'Note that lists created when not logged will not be saved.';
    }
    drupal_set_message(t('The new list %listname was created. %saved',  
        array(
          '%listname' => $createname,
          '%saved' => $savedlist,
          )
        ));
    drupal_goto('pidlist', 'collapsed=' . $templist->listid);
  }
  else {
    drupal_set_message(t('A new list must at least have a name!'), 'error');
  }
}

/**
 * Form constructor for the tab on the object details page.
 * 
 * @global type $user
 * @param type $form_state
 * @param type $pid
 * @see islandora_pid_list_menu()
 */
function islandora_pid_list_pid_form($form_state, $pid) {
  module_load_include('inc', 'php_lib', 'Ahah');
  module_load_include('inc', 'islandora_pid_list', 'islandora_pid_list.api');
  module_load_include('inc', 'islandora_pid_list', 'pidlist');
  drupal_add_css(drupal_get_path('module', 'islandora_pid_list') . '/css/islandora_pid_list.css');
    
  global $user;
  Ahah::get_ahah_js();
  $pidlist = array();
  $pidlist = get_user_pid_list(array('pid' => $pid));

  $form = array(
    '#title' => t('Lists'),
    '#tree' => TRUE,
    '#prefix' => '<div id="listassoc">',
    '#suffix' => '</div>',
    'userlists' => array(
      '#type' => 'fieldset',
      '#title' => t('My Lists'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    ),
    'pid' => array(
      '#type' => 'value',
      '#value' => $pid,
    ), 
  );

  $listids = array();

  // $form['userlists'] = user associated lists (ie lists the user can see).
  $userlistcount = 0;
  foreach ($pidlist as $key => $value) {
    if (in_array($user->uid, $value->userlist)) {
      $userlistcount++;
      $idoutput = str_replace('session_', '', $value->listid);
      $form['userlists'][$key]['label'] = array( 
          '#type' => 'item',
          '#title' => check_plain($value->listname . ' [ID:' . $idoutput . ']'),  
          '#prefix' => '<div class="userlists">',
      );
      if (user_access("Manage other users' pidlists") || $user->uid === $value->listowner) {
        $form['userlists'][$key]['remove'] = array(
          '#type' => 'image_button',
          '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/minus_small.png', 
          '#suffix' => '</div>',
          '#ahah' => array(
            'event' => 'click',
            'path' => "pidlist/ahah/remove/listassociation",
            'wrapper' => "listassoc",
            'method' => 'replace',
          ),
        );
      }
      else {
        $form['userlists'][$key]['remove'] = array(
          '#type' => 'image_button',
          '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/minus_small.png', 
          '#suffix' => '</div>',
          '#ahah' => array(
            'event' => 'click',
            'path' => "pidlist/ahah/remove/listassociation",
            'wrapper' => "listassoc",
            'method' => 'replace',
          ),
        );
      }
      $form['userlists'][$key]['list-id'] = array(
        '#type' => 'value',
        '#value' => $key
      );  

      $form['userlists'][$key]['listtype'] = array(
        '#type' => 'value',
        '#value' => 'userlists',
      );

      $form['userlists'][$key]['plo'] = array(
      '#type' => 'value',
      '#value' => $value,
      );
      array_push($listids, $value->listid);
    }
  }
  if ($userlistcount == 0) {
      $form['userlists']['empty'] = array(
      '#value' => t('No lists currently associated'),
      '#prefix' => '<div class="emptypid">',
      '#suffix' => '</div>',
      );  
    } 

  $optionsuser['default'] = "- Select A List To Add - ";
  $optionsuser['create'] = 'Create a new list';

  $optionsall['default'] = "- Select A List To Add - ";

  $allresult = db_query('SELECT listid, listname FROM {islandora_pid_list_names} ORDER BY listid');
  $userresult = db_query('SELECT listid FROM {islandora_pid_list_lists} WHERE uid = %d ORDER BY listid', $user->uid);

  while ($result = db_fetch_array($allresult)) {
  $optionsall[$result['listid']] = $result['listid'] . ': ' . $result['listname'];
  }

  while ($result = db_fetch_array($userresult)) {
  $optionsuser[$result['listid']] = $optionsall[$result['listid']];
    unset($optionsall[$result['listid']]);
  }

  foreach ($optionsuser as $key => $value) {
    if (in_array($key, $listids)) {
      unset($optionsuser[$key]);
    }
  }

  // Grab from session as well if the user is anon and there exists session objs
  if ($user->uid == 0 && !empty($_SESSION['islandora_pid_list'])) {
    foreach ($_SESSION['islandora_pid_list'] as $value) {
      $sessionlist = unserialize($value);
      $output = str_replace('session_', '', $sessionlist->listid);
        if (!in_array($sessionlist->listid, $listids)) {
          $optionsuser[$sessionlist->listid] = $output . ': ' . $sessionlist->listname;
        }
    }
  }

  $form['userlists']['listoptions'] = array(
    '#type' => 'select',
    '#default_value' => $form_state['post']['userlists']['listoptions'],
    '#options' => $optionsuser,
    '#prefix' => '<div class="addlistassoc">',
    '#suffix' => '</div>',
      '#ahah' => array(
        'event' => 'click',
        'path' => "pidlist/ahah/add/listassociation",
        'wrapper' => "listassoc",
        'method' => 'replace',
        ),
  );
  if ($form_state['post']['userlists']['listoptions'] == 'create') {
    $form['userlists']['addlist'] = array(
      '#type' => 'textfield',
      '#prefix' => '<div class="addnewlist">',
      '#title' => t('Name of new list'),
      '#size' => 20,
      '#weight' => 50000,
      '#ahah' => array(
        'event' => '',
        'keypress' => TRUE,
        'path' => "pidlist/ahah/add/listassociation",
        'wrapper' => "listassoc",
        'method' => 'replace',          
      ),
    );
    $form['userlists']['addlistbutton'] = array(
    '#type' => 'image_button',
    '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/add.png',
    '#suffix' => '</div>',  
    '#weight' => 50001,  
    '#ahah' => array(
        'event' => 'click',
        'path' => "pidlist/ahah/add/listassociation",
        'wrapper' => "listassoc",
        'method' => 'replace',
        ),
    );  

  }

  if (user_access(t('Adminisiter islandora_pid_list module'))) {
    $form['alllists'] = array(
      '#type' => 'fieldset',
      '#title' => t('All Other Lists associated to this PID'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );
    $alllistscount = 0;
    $allids = array();

    foreach ($pidlist as $key => $value) {
      if (!in_array($user->uid, $value->userlist)) {
        array_push($allids, $value->listid);
        $alllistscount++;
        $form['alllists'][$key]['label'] = array( 
          '#type' => 'item',
          '#title' => check_plain($value->listname . ' [ID:' . $value->listid . ']'),  
          '#prefix' => '<div class="userlists">',
        );
        $form['alllists'][$key]['remove'] = array(
          '#type' => 'image_button',
          '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/minus_small.png', 
          '#suffix' => '</div>',
          '#ahah' => array(
            'event' => 'click',
            'path' => "pidlist/ahah/remove/listassociation",
            'wrapper' => "listassoc",
            'method' => 'replace',
          ),
        );

        $form['alllists'][$key]['list-id'] = array(
          '#type' => 'value',
          '#value' => $key
        );  

        $form['alllists'][$key]['plo'] = array(
          '#type' => 'value',
          '#value' => $value,
          );
        }
    }
    if ($alllistscount == 0) {
      $form['alllists']['empty'] = array(
      '#value' => t('No lists currently associated'),
      '#prefix' => '<div class="emptypid">',
      '#suffix' => '</div>',
      );  
    }
  foreach ($optionsall as $key => $value) {
    if (in_array($key, $allids)) {
      unset($optionsall[$key]);
    }
  }
    $form['alllists']['listoptions'] = array(
    '#type' => 'select',
    '#default_value' => $form_state['post']['userlists']['listoptions'],
    '#options' => $optionsall,
    '#prefix' => '<div class="addlistassoc">',
    '#suffix' => '</div>',
      '#ahah' => array(
        'event' => 'click',
        'path' => "pidlist/ahah/add/listassociation",
        'wrapper' => "listassoc",
        'method' => 'replace',
        ),
    );
  }
return $form;
}

/**
 * AHAH callback for adding a pid to a list.
 * 
 * @see islandora_pid_list_menu() 
 */
function islandora_pid_list_add_pid_js() {
  module_load_include('inc', 'php_lib', 'Ahah');
  module_load_include('inc', 'islandora_pid_list', 'pidlist');
  module_load_include('inc', 'islandora_pid_list', 'islandora_pid_list.api');

  list($form_id, $form_build_id, $form, $form_state) = Ahah::getFormInfo();
  
  if (array_key_exists('userlists', $_POST)) {
    foreach ($_POST['userlists'] as $key => $value) {
      if (!empty($value['pids']['add_button'])) {
        $param = 'userlists';
        $listindex = $key;
        $foundpidtext = TRUE;
        break;
      }
    }
  }
  
  if (array_key_exists('alllists', $_POST)) {
    if (!$foundpidtext) {
      foreach ($_POST['alllists'] as $key => $value) {
        if (!empty($value['pids']['add_button'])) {
           $param = 'alllists';
           $listindex = $key;
          break;
        }  
      }
    }
  }
  
  $obj = $form[$param][$listindex]['plo']['#value'];
  $listname = $obj->listname;
  ;  
  // Needed to account for concurrent users making changes on lists.
  $obj = get_pid_list_by_number($obj->listid);
   
  if (islandora_pid_list_object_access($obj, $listname)) {
    $pidindex = 0;
    if (strlen($_POST[$param][$listindex]['pids']['pidtext']) != 0) {
      foreach ($form[$param][$listindex]['pids'] as $item => $assoc) {
        if (is_numeric($item)) {
          $pidindex++;
        }
      }
      $newpid = $_POST[$param][$listindex]['pids']['pidtext'];

      if (!empty($newpid) && !in_array($newpid, $obj->pidlist)) {
        try {
          $obj->add_pids(array($newpid));
          drupal_set_message(t('The pid %newpid has been added to %listname',
          array(
            '%listname' => $obj->listname,
            '%newpid' => $newpid,
          )
      ));  
        } 
        catch (Exception $e) {
          drupal_set_message($e->getMessage(), 'error');
        }
      }
      else {
      drupal_set_message(t("The pid %pid already exists in %listname",
              array(
                '%pid' => $new_pid,
                '%listname' => $obj->listname,
                )
          ), 'error');    
      }
    }
    else {
      $obj = $form[$param][$listindex]['plo']['#value'];
      drupal_set_message(t("Please enter a pid to add to %listname",
                array(
                  '%listname' => $obj->listname,
                  )
            ), 'error');  
    }
  }
  $form = Ahah::rebuildForm($form_id, $form_build_id, $form, $form_state);
  $form[$param][$listindex]['#collapsed'] = FALSE;
  Ahah::respond($form);
  exit();
}

/**
 * AHAH callback for removing a user from a list.
 * 
 * @see islandora_pid_list_menu()
 */
function islandora_pid_list_remove_user_js() {
  module_load_include('inc', 'php_lib', 'Ahah');
  module_load_include('inc', 'islandora_pid_list', 'pidlist');
  module_load_include('inc', 'islandora_pid_list', 'islandora_pid_list.api');
    
  list($form_id, $form_build_id, $form, $form_state) = Ahah::getFormInfo();
    
  $numlists = array(
    'userlists' => count($_POST['userlists']),
    'alllists' => count($_POST['alllists']),
  );
  
  if (array_key_exists('userlists', $_POST)) {
    foreach ($_POST['userlists'] as $key => $value) {
      foreach ($_POST['userlists'][$key] as $item => $index) {
        if ($item == 'users') {
          foreach ($_POST['userlists'][$key][$item] as $offset => $object) {
            if (!empty($object)) {
              $param = 'userlists';
              $listindex = $key;
              $foundusertext = TRUE;
              $useroffset = $offset;
              break;
            }
          }
        }
      }
    }
  }
  
  if (array_key_exists('alllists', $_POST)) {
    if (!$foundusertext) {
      foreach ($_POST['alllists'] as $key => $value) {
        foreach ($_POST['alllists'][$key] as $item => $index) {
          if ($item == 'users') {
            foreach ($_POST['alllists'][$key][$item] as $offset => $object) {
              if (!empty($object)) {
                $param = 'alllists';
                $listindex = $key;
                $foundusertext = TRUE;
                $useroffset = $offset;
                break;
              }
            }
          }
        }
      }
    }
  }
    
  $obj = $form[$param][$listindex]['plo']['#value'];
  $listname = $obj->listname;
    
  // Needed to account for concurrent users making changes on lists.
  $obj = get_pid_list_by_number($obj->listid);
 
  if (islandora_pid_list_object_access($obj, $listname)) {
    $removeuser = $form[$param][$listindex]['users'][$useroffset]['user']['#value'];
   
    if ($removeuser == 0) {
      $outputuser = t('Anonymous');
    }
    else {
    $outputuser = user_load($removeuser)->name;  
    }

    drupal_set_message(t('The user %username has been removed from %listname',
      array(
        '%listname' => $obj->listname,
        '%username' => $outputuser,
      )
    ));   
    $obj->remove_users(array($removeuser));
  }
  $form_state['storage']['listid'] = $obj->listid;
  $form = Ahah::rebuildForm($form_id, $form_build_id, $form, $form_state);
  Ahah::respond($form);
  exit();
}

/**
 * AHAH callback for removing a pid from a list.
 * 
 * @see islandora_pid_list_menu() 
 */
function islandora_pid_list_remove_pid_js() {
  module_load_include('inc', 'php_lib', 'Ahah');
  module_load_include('inc', 'islandora_pid_list', 'pidlist');
  module_load_include('inc', 'islandora_pid_list', 'islandora_pid_list.api');
   
  list($form_id, $form_build_id, $form, $form_state) = Ahah::getFormInfo();
    
  if (array_key_exists('userlists', $_POST)) {
    foreach ($_POST['userlists'] as $key => $value) {
      foreach ($_POST['userlists'][$key] as $item => $index) {
        if ($item == 'pids') {
          foreach ($_POST['userlists'][$key][$item] as $offset => $object) {
            if (!empty($object) && is_array($object)) {
              $param = 'userlists';
              $listindex = $key;
              $foundpidtext = TRUE;
              $pidoffset = $offset;
              break;
            }
          }
        }
      }
    }
  }
  
  if (array_key_exists('alllists', $_POST)) {
    if (!$foundpidtext) {
      foreach ($_POST['alllists'] as $key => $value) {
        foreach ($_POST['alllists'][$key] as $item => $index) {
          if ($item == 'pids') {
            foreach ($_POST['alllists'][$key][$item] as $offset => $object) {
              if (!empty($object) && is_array($object)) {
                $param = 'alllists';
                $listindex = $key;
                $pidoffset = $offset;
                break;
              }
            }
          }
        }
      }
    }
  }
  $obj = $form[$param][$listindex]['plo']['#value'];
  $pidremove = $form[$param][$listindex]['pids'][$pidoffset]['pid']['#value'];
  $listname = $obj->listname;
    
  // Needed to account for concurrent users making changes on lists.
  $obj = get_pid_list_by_number($obj->listid);
 
  if (islandora_pid_list_object_access($obj, $listname)) {
    drupal_set_message(t('The pid %pidname has been removed from %listname',
      array(
        '%listname' => $obj->listname,
        '%pidname' => $pidremove,
      )
    )); 
    $obj->remove_pids(array($pidremove));
  }
    
  $form = Ahah::rebuildForm($form_id, $form_build_id, $form, $form_state);
  $form[$param][$listindex]['#collapsed'] = FALSE;
  Ahah::respond($form);
  exit();
}

/**
 * AHAH callback for adding a pid to a list
 * 
 * @see islandora_pid_list_menu()
 */
function islandora_pid_list_add_user_js() {
  module_load_include('inc', 'php_lib', 'Ahah');
  module_load_include('inc', 'islandora_pid_list', 'pidlist');
  module_load_include('inc', 'islandora_pid_list', 'islandora_pid_list.api');
  
  list($form_id, $form_build_id, $form, $form_state) = Ahah::getFormInfo();
  
  if (array_key_exists('userlists', $_POST)) {
    foreach ($_POST['userlists'] as $key => $value) {
      foreach ($_POST['userlists'][$key] as $item => $index) {
        if ($item == 'users') {
          if (is_array($index)) {
            if (array_key_exists('listadd', $index)) {
              $param = 'userlists';
              $listindex = $key;
              $adduser = $index['listusers'];
              $foundselect = TRUE;
              break;
            }  
          }
        }
      } 
    }
  }
      
  if (array_key_exists('alllists', $_POST)) {
    if (!$foundselect) {
      foreach ($_POST['alllists'] as $key => $value) {
        foreach ($_POST['alllists'][$key] as $item => $index) {
          if ($item == 'users') {
            if (is_array($index)) {
              if (array_key_exists('listadd', $index)) {
                $param = 'alllists';
                $listindex = $key;
                $adduser = $index['listusers'];
                break;
              }  
            }
          }
        } 
      }
    }
  }
  $obj = $form[$param][$listindex]['plo']['#value'];
  $listname = $obj->listname;
    
  // Needed to account for concurrent users making changes on lists.
  $obj = get_pid_list_by_number($obj->listid);
 
  if (islandora_pid_list_object_access($obj, $listname)) {
    if (strlen($adduser)) {
      if ($adduser == 0) {
        $outputuser = t('Anonymous');
      }
      else {
        $outputuser = user_load($adduser)->name;  
      } 
      drupal_set_message(t('The user %adduser has been added to %listname',
        array(
          '%listname' => $obj->listname,
          '%adduser' => $outputuser,
        )
      )); 
      $obj->add_users(array($adduser));
    }
    else {
      drupal_set_message(t("Please select a user to add to %listname",
        array(
          '%listname' => $obj->listname,
          )
      ), 'error');  
    }
  }
  
  $form_state['storage']['listid'] = $obj->listid;
  $form = Ahah::rebuildForm($form_id, $form_build_id, $form, $form_state);
  Ahah::respond($form);
  exit();
}

/**
 * AHAH callback from removing a pid from a list.
 * 
 * @see islandora_pid_list_menu() 
 */
function islandora_pid_list_remove_list_js() {
  module_load_include('inc', 'php_lib', 'Ahah');
  module_load_include('inc', 'islandora_pid_list', 'pidlist');
  module_load_include('inc', 'islandora_pid_list', 'islandora_pid_list.api');
  
  list($form_id, $form_build_id, $form, $form_state) = Ahah::getFormInfo();
  
  if (array_key_exists('userlists', $_POST)) {
    foreach ($_POST['userlists'] as $key => $value) {
      if ($_POST['userlists'][$key]['remove']) {
          $param = 'userlists';
          $foundremove = TRUE;
          $listindex = $key;
          break;
      }
    } 
  }
      
  if (array_key_exists('alllists', $_POST)) {
    if (!$foundremove) {
      foreach ($_POST['alllists'] as $key => $value) {
        if ($_POST['alllists'][$key]['remove']) {
          $param = 'alllists';
          $listindex = $key;
          break;
        }  
      }
    }
  }
  
  $obj = $form[$param][$listindex]['plo']['#value'];
  $listname = $obj->listname;
    
  // Needed to account for concurrent users making changes on lists.
  $obj = get_pid_list_by_number($obj->listid);
 
  if (islandora_pid_list_object_access($obj, $listname)) {
    drupal_set_message(t('The list %listname has been deleted',
            array(
              '%listname' => $obj->listname,
            )
    ));    
    $obj->remove_list();
  }  
  $form_state['storage']['listid'] = $obj->listid;
  $form = Ahah::rebuildForm($form_id, $form_build_id, $form, $form_state);
  Ahah::respond($form);
  exit();
}

/**
 * AHAH callback for updating the name of a list.
 * 
 * @see islandora_pid_list_menu() 
 */
function islandora_pid_list_update_name_js() {
  module_load_include('inc', 'php_lib', 'Ahah');
  module_load_include('inc', 'islandora_pid_list', 'pidlist');
  module_load_include('inc', 'islandora_pid_list', 'islandora_pid_list.api');
  
  list($form_id, $form_build_id, $form, $form_state) = Ahah::getFormInfo();
  
  if (array_key_exists('userlists', $_POST)) {
    foreach ($_POST['userlists'] as $key => $value) {
      $param = 'userlists';
      $listindex = $key;
      $obj = $form[$param][$listindex]['plo']['#value'];
      $newlistname = $_POST[$param][$listindex]['listname'];
      
      if ($_POST['userlists'][$key]['changename'] || $obj->listname != $newlistname) {
        $foundchange = TRUE;
        break;
      }
    } 
  }
      
  if (array_key_exists('alllists', $_POST)) {
    if (!$foundchange) {
      foreach ($_POST['alllists'] as $key => $value) {
        $param = 'alllists';
        $listindex = $key;
        $obj = $form[$param][$listindex]['plo']['#value'];
        $newlistname = $_POST[$param][$listindex]['listname'];
        
        if ($_POST['alllists'][$key]['changename'] || $obj->listname != $newlistname) {
          break;
        }  
      }
    }
  }
  
  $listname = $obj->listname;
    
  // Needed to account for concurrent users making changes on lists.
  $obj = get_pid_list_by_number($obj->listid);
 
  if (islandora_pid_list_object_access($obj, $listname)) {
    if ($obj->listname != $newlistname) {
      if (ctype_space($newlistname) || $newlistname == '') {
        drupal_set_message(t('The list %listname must have a name!',
            array(
            '%listname' => $obj->listname,  
            )
        ), 'error');
      }
      else {
        drupal_set_message(t('The list %beforelist has had its name changed to %afterlist',
            array(
              '%beforelist' => $obj->listname,
              '%afterlist' => $newlistname,
            )
        ));    
        $obj->change_list_name($newlistname);    
      }
    }
  }
   
  $form = Ahah::rebuildForm($form_id, $form_build_id, $form, $form_state);
  $form[$param][$listindex]['#collapsed'] = FALSE;
  Ahah::respond($form);
  exit();
}

/**
 * AHAH callback for adding adding a list association to a pid.
 * 
 * @global int $user 
 * 
 * @see islandora_pid_list_menu()
 */
function islandora_pid_list_add_association_js() {
  module_load_include('inc', 'php_lib', 'Ahah');
  module_load_include('inc', 'islandora_pid_list', 'pidlist');
  module_load_include('inc', 'islandora_pid_list', 'islandora_pid_list.api');
  
  global $user;
  
  if (array_key_exists('userlists', $_POST) && $_POST['userlists']['listoptions'] != 'default') {
    $param = 'userlists';
  }
  if (array_key_exists('alllists', $_POST) && $_POST['alllists']['listoptions'] != 'default') {
    $param = 'alllists';
  }
  
  list($form_id, $form_build_id, $form, $form_state) = Ahah::getFormInfo();
  
  if ($param) {
    if ($_POST['userlists']['listoptions'] == 'create') {
      if (array_key_exists('addlist', $_POST['userlists'])) {
          $addlist = $_POST['userlists']['addlist'];
          if (!ctype_space($addlist) && $addlist != '') {
            $created = TRUE;
            $paramusers = array();
            $parampids = array();

            $params = array(
              'retrieve' => FALSE,
              'useruid' => $user->uid,
              'listowner' => $user->uid,
              'listname' => $addlist,
              'listusers' => array($user->uid),
            );

            if ($user->uid == 0) {
              $params['session'] = TRUE;
            }

            $templist = new PidList($params);

            try {
               $templist->add_pids(array($form['pid']['#value']));
            } 
            catch (Exception $e) {
                drupal_set_message($e->getMessage() . ' The list was still created!', 'error');
            }

            if ($user->uid == 0) {
              $savedlist = 'Note that lists created when not logged will not be saved.';
            }
            drupal_set_message(t('The new list %listname was created. %saved',  
                array(
                  '%listname' => $addlist,
                  '%saved' => $savedlist,
                  )
                ));
          }
          else {
            drupal_set_message(t('A new list must at least have a name!'), 'error');
          }
        }
    }
    elseif ($_POST[$param]['listoptions'] != 'default') {
      $list = get_pid_list_by_number($_POST[$param]['listoptions']);
      
        if (islandora_pid_list_object_access($list, $list->listname)) {
          $addpid = $form['pid']['#value'];
          $list->add_pids(array($addpid));

          drupal_set_message(t('The pid %addpid has added to %listname',
              array(
                '%listname' => $list->listname,
                '%addpid' => $addpid,
              )
          ));  
        }
    }
    if ($created) {
        $form_state['post'][$param]['listoptions'] = 'default';
    }
  }   
  $form = Ahah::rebuildForm($form_id, $form_build_id, $form, $form_state);
  $form[$param][$listindex]['#collapsed'] = FALSE;
  Ahah::respond($form);
  exit();
}

/**
 * AHAH callback for removing a list association to a pid.
 * 
 * @see islandora_pid_list_menu() 
 */
function islandora_pid_list_remove_association_js() {
  module_load_include('inc', 'php_lib', 'Ahah');
  module_load_include('inc', 'islandora_pid_list', 'islandora_pid_list.api');
  module_load_include('inc', 'islandora_pid_list', 'pidlist');
  global $user;
  
  list($form_id, $form_build_id, $form, $form_state) = Ahah::getFormInfo();
  
  if (array_key_exists('userlists', $_POST)) {
    foreach ($_POST['userlists'] as $key => $value) {
      if (is_numeric($key) && array_key_exists('remove', $_POST['userlists'][$key])) {
          $param = 'userlists';
          $foundremove = TRUE;
          $listindex = $key;
            break;
      }
    } 
  }
      
  if (array_key_exists('alllists', $_POST)) {
    if (!$foundremove) {
      foreach ($_POST['alllists'] as $key => $value) {
        if (is_numeric($key) && array_key_exists('remove', $_POST['alllists'][$key])) {
          $param = 'alllists';
          $listindex = $key;
          break;
        }
      }
    }
  }
   
  $obj = $form[$param][$listindex]['plo']['#value'];
  $listname = $obj->listname;
    
  // Needed to account for concurrent users making changes on lists.
  $obj = get_pid_list_by_number($obj->listid);
 
  if (islandora_pid_list_object_access($obj, $listname)) {
    if (user_access("Manage other users' pidlists") || $user->uid === $obj->listowner) {
      $removepid = $form['pid']['#value'];
      $obj->remove_pids(array($removepid));

      drupal_set_message(t('The pid %removepid has removed from %listname',
        array(
          '%listname' => $obj->listname,
          '%removepid' => $removepid,
        )
      ));
    }
    else {
      $obj->remove_users(array($user->uid));
      drupal_set_message(t('You have removed yourself from %listname',
          array(
            '%listname' => $obj->listname,
          )
      ));
    }
  }
  
  $form = Ahah::rebuildForm($form_id, $form_build_id, $form, $form_state);
  $form[$param][$listindex]['#collapsed'] = FALSE;
  Ahah::respond($form);
  exit();
}

/**
 * Checks to see if a user modified the specified list concurrently.
 * If the current user was removed from the list or the list itself was removed
 * this function will return false and display errors.
 * 
 * @global integer $user
 *
 * @param PidList $obj
 *   The PidList object that is being checked.
 * 
 * @param string $listname
 *   Name of the list, used for output to the user.
 * 
 * @return boolean 
 */
function islandora_pid_list_object_access(PidList $obj, $listname) {
  global $user;
    
  if (!$obj->session) {
    if (is_null($obj->listname)) {
      drupal_set_message(t("The list %listname no longer exists.",
          array(
            '%listname' => $listname,
            )
        ), 'error'); 
      return FALSE;
    }
    elseif (in_array($user->uid, $obj->userlist) || user_access('Adminisiter islandora_pid_list module')) {
      return TRUE;
    }
    else {
      drupal_set_message(t("You no longer have access to the list %listname",
          array(
            '%listname' => $obj->listname,
            )
        ), 'error');
      return FALSE;
    }
  }
  else {
    return TRUE;
  }
}