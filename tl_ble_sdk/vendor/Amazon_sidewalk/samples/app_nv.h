/*
 * app_nv.h
 *
 *  Created on: 2025.5
 *      Author: Admin
 */

#ifndef VENDOR_TAGSDK_EXAMPLE_TELINK_APP_NV_H_
#define VENDOR_TAGSDK_EXAMPLE_TELINK_APP_NV_H_

int app_tag_stroage_set_data(int idx, u8* data, size_t len, u8 type);

int app_tag_stroage_get_data(int idx, u8* data, size_t len, u8 type);

int app_tag_stroage_del_data(int idx, u8 type);

void app_stoage_init(u8 type);

int app_tag_get_data_len(int idx, u8 type);

int app_tag_stroage_del_by_prefix(int idx, u8 type);

#endif /* VENDOR_TAGSDK_EXAMPLE_TELINK_APP_NV_H_ */
