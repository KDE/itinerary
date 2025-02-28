#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.0-or-later
# SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>

from itinerarytestcase import ItineraryTestCase

import unittest
from appium.webdriver.common.appiumby import AppiumBy
from selenium.webdriver.common.keys import Keys


class SettingsTest(ItineraryTestCase):
    def test_settings(self):
        self.openGlobalDrawer()
        self.driver.find_element(by=AppiumBy.NAME, value="Settings…").click()

        # we run in en-US locale, so the metric unit switch has to exist
        metricSwitch = self.driver.find_element(by=AppiumBy.NAME, value="Use metric units")
        self.assertTrue(metricSwitch.is_displayed())
        metricSwitch.click()

        trafficData = self.driver.find_element(by=AppiumBy.NAME, value="Query Traffic Data")
        self.assertEqual(trafficData.get_attribute("checked"), "false")
        trafficData.send_keys(Keys.SPACE)  # TODO there is no toggle()?
        self.assertEqual(trafficData.get_attribute("checked"), "true")

        self.driver.find_element(by=AppiumBy.NAME, value="Public Transport Information Sources…").click()
        self.goBack()
        self.goBack()

        self.openGlobalDrawer()
        self.driver.find_element(by=AppiumBy.NAME, value="Settings…").click()
        self.assertEqual(self.driver.find_element(by=AppiumBy.NAME, value="Query Traffic Data").get_attribute("checked"), "true")
        self.goBack()


if __name__ == '__main__':
    unittest.main()
