#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.0-or-later
# SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>

from itinerarytestcase import ItineraryTestCase

import unittest
from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
import selenium.common.exceptions
from selenium.webdriver.common.action_chains import ActionChains
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support.ui import WebDriverWait

import os

class ReservationTest(ItineraryTestCase):
    def setUp(self):
        # open the timeline page and import test scenario
        self.driver.find_element(by=AppiumBy.NAME, value="Itinerary").click()
        self.triggerImportAction("Open File...")
        self.openFile("../tests/randa2017.json")
        self.driver.find_element(by=AppiumBy.NAME, value="Trip: Randa (September 2017)").click()

    def tearDown(self):
        self.driver.find_element(by=AppiumBy.NAME, value="Delete trip").click()
        deleteDialog = self.driver.find_element(by=AppiumBy.NAME, value="Delete Trip")
        self.assertTrue(deleteDialog.is_displayed())
        deleteDialog.find_element(by=AppiumBy.NAME, value="Delete").click()

    def test_flight(self):
        self.driver.find_element(by=AppiumBy.NAME, value="LX 963 TXL → ZRH").click()
        self.assertFalse(self.driver.find_element(by=AppiumBy.NAME, value="Barcode Scan Mode").is_displayed())
        self.assertTrue(self.driver.find_element(by=AppiumBy.NAME, value="No documents attached to this reservation.").is_enabled())
        self.driver.find_element(by=AppiumBy.NAME, value="Add Document...").click()
        self.openFile("data/iata-bcbp-demo.pdf")
        self.assertTrue(self.driver.find_element(by=AppiumBy.NAME, value="iata-bcbp-demo.pdf").is_enabled())
        # TODO document context actions not accessible
        self.driver.find_element(by=AppiumBy.NAME, value="Edit").click()
        self.goBack()
        self.goBack()

    def test_train(self):
        self.driver.find_element(by=AppiumBy.NAME, value="IC 816").click()
        self.assertTrue(self.driver.find_element(by=AppiumBy.NAME, value="Barcode Scan Mode").is_enabled())
        # TODO how to check those *don't* exist?
        # self.assertFalse(self.driver.find_element(by=AppiumBy.NAME, value="Departure Vehicle Layout").is_enabled())
        # self.assertFalse(self.driver.find_element(by=AppiumBy.NAME, value="Arrival Vehicle Layout").is_enabled())
        # self.assertTrue(self.driver.find_element(by=AppiumBy.NAME, value="Journey Details").is_enabled())
        # TODO those two aren't found for some reason??
        # self.assertFalse(self.driver.find_element(by=AppiumBy.NAME, value="Add transfer before").is_enabled())
        # self.assertFalse(self.driver.find_element(by=AppiumBy.NAME, value="Add transfer after").is_enabled())
        self.assertTrue(self.driver.find_element(by=AppiumBy.NAME, value="Alternatives").is_enabled())
        self.driver.find_element(by=AppiumBy.NAME, value="Edit").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Save").click()
        self.goBack()
        self.driver.find_element(by=AppiumBy.NAME, value="R 241").click()
        # self.assertEqual(len(self.driver.find_elements(by=AppiumBy.NAME, value="View Indoor Map")), 2)
        self.goBack()

if __name__ == '__main__':
    unittest.main()
