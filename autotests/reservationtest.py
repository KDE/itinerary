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

        self.driver.find_element(by=AppiumBy.NAME, value="Import").click()
        self.driver.find_element(by=AppiumBy.NAME, value="From file").click()

        self.openFile("../tests/randa2017.json")
        self.driver.find_element(by=AppiumBy.NAME, value="Import selection").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Randa (September 2017)").click()

    def tearDown(self):
        self.driver.find_element(by=AppiumBy.NAME, value="Delete trip").click()
        deleteDialog = self.driver.find_element(by=AppiumBy.NAME, value="Delete Trip")
        self.assertTrue(deleteDialog.is_displayed())
        deleteDialog.find_element(by=AppiumBy.NAME, value="Delete").click()

    def test_flight(self):
        self.driver.find_element(by=AppiumBy.NAME, value="LX 963 TXL → ZRH").click()
        self.assertFalse(self.driver.find_element(by=AppiumBy.NAME, value="Barcode Scan Mode").is_displayed())
        self.assertTrue(self.driver.find_element(by=AppiumBy.NAME, value="No documents attached to this reservation.").is_enabled())
        self.driver.find_element(by=AppiumBy.NAME, value="Add Document…").click()
        self.openFile("data/iata-bcbp-demo.pdf")
        self.assertTrue(self.driver.find_element(by=AppiumBy.NAME, value="iata-bcbp-demo.pdf").is_enabled())
        # TODO document context actions not accessible
        self.driver.find_element(by=AppiumBy.NAME, value="Edit").click()
        self.assertTrue(self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[panel | Edit Flight]").is_displayed())
        self.goBack()

        page = self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[panel | Flight]")
        self.assertTrue(page.is_displayed())
        exportAction = page.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Export…]")
        self.assertTrue(exportAction.is_enabled())
        exportAction.click()

        exportDialog = self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[dialog | Export Reservation]")
        self.assertTrue(exportDialog)
        exportDialog.find_element(by=AppiumBy.NAME, value="As Itinerary file…").click()
        self.openFile("export-test.itinerary")

        deleteAction = page.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Delete…]")
        self.assertTrue(deleteAction.is_enabled())
        deleteAction.click()

        deleteDialog = self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[dialog | Delete Event]")
        self.assertTrue(deleteDialog.is_displayed())
        deleteDialog.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Cancel]").click()
        deleteAction.click()
        deleteDialog = self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[dialog | Delete Event]")
        deleteDialog.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Delete]").click()

    def test_train(self):
        # TODO should be IC 816, but that is consistently set to Accessible.ignored = true for unknown reasons...
        self.driver.find_element(by=AppiumBy.NAME, value="R 241").click()
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
        page = self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[panel | Edit Train Reservation]")
        self.assertTrue(page.is_displayed())
        page.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Save]").click()

        page = self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[panel | Train Ticket]")
        self.assertTrue(page.is_displayed())
        page.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Station]").click()
        self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[push button | View Indoor Map]").click()
        self.assertTrue(self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[panel | Floor 1]").is_displayed())
        self.goBack()

        page.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Station]").click()
        self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Public Transport Departures]").click()
        self.assertTrue(self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[panel | Departures]").is_displayed())
        self.goBack()

        page.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Alternatives]").click()
        self.assertTrue(self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[panel | Alternative Connections]").is_displayed())
        self.goBack()

        self.assertTrue(page.is_displayed())
        self.assertTrue(self.driver.find_element(by=AppiumBy.NAME, value="Visp to Randa").is_displayed())
        self.goBack()
        # TODO we need to scroll down for this, how to do that?
        # self.driver.find_element(by=AppiumBy.NAME, value="R 241").click()
        # self.assertEqual(len(self.driver.find_elements(by=AppiumBy.NAME, value="View Indoor Map")), 2)
        # self.goBack()


class ReservationCreationTest(ItineraryTestCase):
    def setUp(self):
        self.driver.find_element(by=AppiumBy.NAME, value="Itinerary").click()
        self.createAndEnterTrip("My New Trip")

    def tearDown(self):
        self.driver.find_element(by=AppiumBy.NAME, value="Delete trip").click()
        deleteDialog = self.driver.find_element(by=AppiumBy.NAME, value="Delete Trip")
        self.assertTrue(deleteDialog.is_displayed())
        deleteDialog.find_element(by=AppiumBy.NAME, value="Delete").click()

    def test_bus(self):
        page = self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[panel | My New Trip]")
        self.assertTrue(page.is_displayed())
        newAction = page.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Add bus]")
        self.assertTrue(newAction.is_enabled())
        newAction.click()

        editor = self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[panel | New Bus Reservation]")
        self.assertTrue(editor.is_displayed())
        saveButton = editor.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Save]")
        self.assertFalse(saveButton.is_enabled())
        editor.find_elements(by=AppiumBy.NAME, value="Stop Name")[0].send_keys("Testdeparturebusstop")
        editor.find_elements(by=AppiumBy.NAME, value="Stop Name")[1].send_keys("Testarrivalbusstop")
        self.assertTrue(saveButton.is_enabled())
        saveButton.click()

        delegate = page.find_element(AppiumBy.CLASS_NAME, value="[filler | Testdeparturebusstop to Testarrivalbusstop]")
        self.assertTrue(delegate.is_displayed())
        delegate.click()
        self.assertTrue(self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[panel | Bus Ticket]").is_displayed())
        self.goBack()

    def test_ferry(self):
        page = self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[panel | My New Trip]")
        self.assertTrue(page.is_displayed())
        newAction = page.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Add ferry trip]")
        self.assertTrue(newAction.is_enabled())
        newAction.click()

        editor = self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[panel | New Boat Trip]")
        self.assertTrue(editor.is_displayed())
        saveButton = editor.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Save]")
        self.assertFalse(saveButton.is_enabled())
        editor.find_elements(by=AppiumBy.NAME, value="Terminal Name")[0].send_keys("Testdepartureharbor")
        editor.find_elements(by=AppiumBy.NAME, value="Terminal Name")[1].send_keys("Testarrivalharbor")
        self.assertTrue(saveButton.is_enabled())
        saveButton.click()

        delegate = page.find_element(AppiumBy.CLASS_NAME, value="[filler | Testdepartureharbor to Testarrivalharbor]")
        self.assertTrue(delegate.is_displayed())
        delegate.click()
        self.assertTrue(self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[panel | Boat Ticket]").is_displayed())
        self.goBack()

    def test_createHotel(self):
        page = self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[panel | My New Trip]")
        self.assertTrue(page.is_displayed())
        newAction = page.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Add accommodation]")
        self.assertTrue(newAction.is_enabled())
        newAction.click()

        editor = self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[panel | New Hotel Reservation]")
        self.assertTrue(editor.is_displayed())
        saveButton = editor.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Save]")
        self.assertFalse(saveButton.is_enabled())
        editor.find_element(by=AppiumBy.NAME, value="Name").send_keys("Testhotel")
        self.assertTrue(saveButton.is_enabled())
        saveButton.click()

        delegate = page.find_element(AppiumBy.CLASS_NAME, value="[filler | Testhotel]")
        self.assertTrue(delegate.is_displayed())
        delegate.click()
        self.assertTrue(self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[panel | Hotel Reservation]").is_displayed())
        self.goBack()
        self.assertTrue(page.find_element(AppiumBy.NAME, value="Check-out Testhotel").is_displayed())

    def test_createRestaurent(self):
        page = self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[panel | My New Trip]")
        self.assertTrue(page.is_displayed())
        newAction = page.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Add restaurant]")
        self.assertTrue(newAction.is_enabled())
        newAction.click()

        editor = self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[panel | New Restaurant]")
        self.assertTrue(editor.is_displayed())
        saveButton = editor.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Save]")
        self.assertFalse(saveButton.is_enabled())
        editor.find_element(by=AppiumBy.NAME, value="Name").send_keys("Testrestaurant")
        self.assertTrue(saveButton.is_enabled())
        saveButton.click()

        delegate = page.find_element(AppiumBy.CLASS_NAME, value="[filler | Testrestaurant]")
        self.assertTrue(delegate.is_displayed())
        delegate.click()
        self.assertTrue(self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[panel | Restaurant Reservation]").is_displayed())
        self.goBack()


if __name__ == '__main__':
    unittest.main()
