<?php

/**
 * EMS shipping settings form
 */
function uc_ems_admin_settings() {
  $form = array();
  $pickup_addr_uc = (array) variable_get('uc_quote_store_default_address', new stdClass());
  $pickup_addr_ems = _uc_ems_get_address($pickup_addr_uc);
  
  $form['pickup'] = array(
    '#type' => 'item',
    '#title' => 'Pickup address status',
    '#value' => $pickup_addr_ems && $pickup_addr_uc['country'] == 643 ?
      '<span style="color:green; font-weight:bold">'. t('Completed') .'</span> EMS ID=' . $pickup_addr_ems :
      '<span style="color:red; font-weight:bold">'. t('Not completed!') .'</span>',
    '#description' => t('EMS shipping will be active only if default pickup address
      was filled on <a href="@url">Shipping quote settings</a> page and it is located in Russia',
      array('@url' => url('admin/store/settings/quotes/edit'))),
  );
  
  $form['uc_ems_extra_charge_type'] = array(
    '#type' => 'select',
    '#options' => array('percent'=>'percent', 'fixed'=>'fixed'),
    '#title' => t('Extra charge type'),
    '#default_value' => variable_get('uc_ems_extra_charge_type', 'percent'),
    '#description' => t('Adds extra amount to calculated shipping price'),
  );
  
  $form['uc_ems_extra_charge'] = array(
    '#type' => 'textfield',
    '#title' => t('Extra charge amount'),
    '#description' => t('Numeric value, without "%" and other symbols. Leave 0 to disable extra charges.'),
    '#default_value' => variable_get('uc_ems_extra_charge', 0),
    '#size' => 15,
    '#field_suffix' =>
      variable_get('uc_ems_extra_charge_type', 'percent') == 'percent' ? '%' : 'р.',
  );
  $form['uc_ems_city_cleanup'] = array(
    '#type' => 'textfield',
    '#title' => t('Remove these words/symbols from city'),
    '#default_value' => variable_get('uc_ems_city_cleanup', 'г.|пос.'),
    '#description' => t('Comma-separated words to remove from customer entered city name'),
  );
  $form['uc_ems_fixed_price'] = array(
    '#type' => 'textfield',
    '#title' => t('Fixed delivery price'),
    '#default_value' => variable_get('uc_ems_fixed_price', 1000),
    '#description' => t('If EMS website is down, this price is used as a EMS delivery price. Leave 0 to not use fixed price approach (in that case error is shown to user during calculating delivery price.'),
    '#size' => 15,
    '#field_suffix' => 'р.',
  );
  $form['uc_ems_insurance'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add EMS insurance to delivery price'),
    '#default_value' => variable_get('uc_ems_insurance', TRUE),
    '#description' => t('Adds 1% of order total to delivery price.'),
  );
  $form['uc_ems_default_weight'] = array(
    '#type' => 'textfield',
    '#title' => t('Default product weight'),
    '#default_value' => variable_get('uc_ems_default_weight', 0),
    '#description' => t('EMS requires weight for each item. That means, that if
     you forgot to enter weight in some product, shipping calculation won\'t work. You can specify default product weight here to avoid that behavior. Leave 0 to make this feature inactive.'),
    '#size' => 10,
    '#field_suffix' => 'kg',
  );
  
  return system_settings_form($form);
}