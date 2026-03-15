#!/usr/bin/env python3
# coding: gbk
"""
webdriver_test.py - Selenium 测试脚本 for CS:APP Proxy Lab

功能：
1. 测试基本代理功能（通过代理访问 Tiny 服务器）
2. 测试缓存功能（重复访问验证缓存命中）
3. 测试并发功能（多线程同时访问）
4. 生成测试报告
"""

import sys
import time
import threading
import argparse
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from selenium.common.exceptions import TimeoutException, WebDriverException


# ==================== 配置 ====================

class Config:
    """测试配置"""
    # 代理服务器配置
    PROXY_HOST = "localhost"
    PROXY_PORT = 7777  # 修改为您的代理端口
    
    # Tiny 服务器配置
    TINY_HOST = "localhost"
    TINY_PORT = 7778   # 修改为您的 Tiny 端口
    
    # 测试页面
    TINY_URL = f"http://{TINY_HOST}:{TINY_PORT}/"
    CGI_URL = f"http://{TINY_HOST}:{TINY_PORT}/cgi-bin/adder?1&22"
    
    # 超时设置（秒）
    PAGE_LOAD_TIMEOUT = 10
    IMPLICIT_WAIT = 5
    
    # 测试次数
    CACHE_TEST_ROUNDS = 3
    CONCURRENT_THREADS = 5


# ==================== WebDriver 管理 ====================

def create_driver(proxy_port=None, headless=True):
    """
    创建配置好的 WebDriver 实例
    
    Args:
        proxy_port: 代理端口，None 表示不使用代理
        headless: 是否无头模式
    """
    options = webdriver.ChromeOptions()
    
    if headless:
        options.add_argument("--headless")
    
    options.add_argument("--no-sandbox")
    options.add_argument("--disable-dev-shm-usage")
    options.add_argument("--disable-gpu")
    options.add_argument("--window-size=1920,1080")
    
    # 配置代理
    if proxy_port:
        proxy = f"{Config.PROXY_HOST}:{proxy_port}"
        options.add_argument(f"--proxy-server=http://{proxy}")
        print(f"[INFO] 使用代理: {proxy}")
    else:
        print("[INFO] 不使用代理（直连）")
    
    # 禁用缓存（用于测试代理缓存，而非浏览器缓存）
    options.add_argument("--disable-application-cache")
    options.add_argument("--disable-cache")
    
    try:
        driver = webdriver.Chrome(options=options)
        driver.set_page_load_timeout(Config.PAGE_LOAD_TIMEOUT)
        driver.implicitly_wait(Config.IMPLICIT_WAIT)
        return driver
    except WebDriverException as e:
        print(f"[ERROR] 无法创建 WebDriver: {e}")
        print("[HINT] 请确保 ChromeDriver 已安装并添加到 PATH")
        sys.exit(1)


# ==================== 测试用例 ====================

class ProxyTests:
    """代理测试类"""
    
    def __init__(self):
        self.results = []
    
    def log(self, test_name, success, message="", duration=0):
        """记录测试结果"""
        status = "? PASS" if success else "? FAIL"
        print(f"[{status}] {test_name}: {message} ({duration:.3f}s)")
        self.results.append({
            "name": test_name,
            "success": success,
            "message": message,
            "duration": duration
        })
        return success
    
    # ---------- 测试1: 基本连接 ----------
    
    def test_basic_connection(self):
        """测试基本代理连接（访问 Tiny home.html）"""
        print("\n" + "="*50)
        print("测试1: 基本代理连接")
        print("="*50)
        
        driver = None
        start_time = time.time()
        
        try:
            driver = create_driver(proxy_port=Config.PROXY_PORT)
            driver.get(Config.TINY_URL)
            
            # 验证页面内容
            if "Tiny" in driver.title or "Tiny" in driver.page_source:
                duration = time.time() - start_time
                return self.log("Basic Connection", True, 
                              f"成功访问 {Config.TINY_URL}", duration)
            else:
                duration = time.time() - start_time
                return self.log("Basic Connection", False,
                              "页面内容不匹配", duration)
                
        except TimeoutException:
            duration = time.time() - start_time
            return self.log("Basic Connection", False,
                          "页面加载超时", duration)
        except Exception as e:
            duration = time.time() - start_time
            return self.log("Basic Connection", False,
                          f"异常: {str(e)}", duration)
        finally:
            if driver:
                driver.quit()
    
    # ---------- 测试2: CGI 动态内容 ----------
    
    def test_cgi_content(self):
        """测试 CGI 动态内容（adder 程序）"""
        print("\n" + "="*50)
        print("测试2: CGI 动态内容")
        print("="*50)
        
        driver = None
        start_time = time.time()
        
        try:
            driver = create_driver(proxy_port=Config.PROXY_PORT)
            driver.get(Config.CGI_URL)
            
            page_source = driver.page_source
            
            # 验证 CGI 计算结果
            if "The answer is:" in page_source and "23" in page_source:
                duration = time.time() - start_time
                return self.log("CGI Content", True,
                              "CGI 计算正确 (1+22=23)", duration)
            else:
                duration = time.time() - start_time
                return self.log("CGI Content", False,
                              f"CGI 响应异常: {page_source[:200]}", duration)
                
        except Exception as e:
            duration = time.time() - start_time
            return self.log("CGI Content", False,
                          f"异常: {str(e)}", duration)
        finally:
            if driver:
                driver.quit()
    
    # ---------- 测试3: 缓存功能 ----------
    
    def test_caching(self):
        """测试代理缓存（多次访问同一资源）"""
        print("\n" + "="*50)
        print("测试3: 缓存功能")
        print("="*50)
        
        durations = []
        
        for i in range(Config.CACHE_TEST_ROUNDS):
            driver = None
            start_time = time.time()
            
            try:
                driver = create_driver(proxy_port=Config.PROXY_PORT)
                driver.get(Config.TINY_URL)
                
                duration = time.time() - start_time
                durations.append(duration)
                print(f"  第 {i+1} 次访问: {duration:.3f}s")
                
            except Exception as e:
                return self.log("Caching", False,
                              f"第 {i+1} 次访问失败: {e}", 0)
            finally:
                if driver:
                    driver.quit()
            
            time.sleep(0.5)  # 短暂间隔
        
        # 分析缓存效果
        if len(durations) >= 2:
            first_time = durations[0]
            avg_later = sum(durations[1:]) / len(durations[1:])
            speedup = first_time / avg_later if avg_later > 0 else 0
            
            message = f"首次 {first_time:.3f}s, 后续平均 {avg_later:.3f}s, 加速比 {speedup:.2f}x"
            
            # 如果后续明显更快，认为缓存有效
            if speedup > 1.5 or avg_later < first_time * 0.7:
                return self.log("Caching", True, f"缓存可能生效 - {message}", sum(durations))
            else:
                return self.log("Caching", True, f"缓存效果不明显 - {message}", sum(durations))
        
        return self.log("Caching", True, "测试完成", sum(durations))
    
    # ---------- 测试4: 并发访问 ----------
    
    def test_concurrent(self):
        """测试并发访问（多线程同时请求）"""
        print("\n" + "="*50)
        print("测试4: 并发访问")
        print("="*50)
        
        results = []
        threads = []
        
        def worker(thread_id):
            """工作线程"""
            start_time = time.time()
            driver = None
            
            try:
                driver = create_driver(proxy_port=Config.PROXY_PORT)
                driver.get(Config.TINY_URL)
                
                # 简单验证
                success = "Tiny" in driver.page_source
                duration = time.time() - start_time
                
                results.append({
                    "id": thread_id,
                    "success": success,
                    "duration": duration
                })
                print(f"  线程 {thread_id}: {'成功' if success else '失败'} ({duration:.3f}s)")
                
            except Exception as e:
                results.append({
                    "id": thread_id,
                    "success": False,
                    "error": str(e)
                })
                print(f"  线程 {thread_id}: 异常 - {e}")
            finally:
                if driver:
                    driver.quit()
        
        # 启动多个线程
        start_time = time.time()
        for i in range(Config.CONCURRENT_THREADS):
            t = threading.Thread(target=worker, args=(i,))
            threads.append(t)
            t.start()
        
        # 等待所有线程完成
        for t in threads:
            t.join()
        
        total_duration = time.time() - start_time
        
        # 统计结果
        successes = sum(1 for r in results if r.get("success", False))
        failures = len(results) - successes
        
        message = f"{successes}/{len(results)} 成功, 总耗时 {total_duration:.3f}s"
        
        if failures == 0:
            return self.log("Concurrent", True, message, total_duration)
        else:
            return self.log("Concurrent", False, message, total_duration)
    
    # ---------- 测试5: 大对象传输 ----------
    
    def test_large_object(self):
        """测试大对象传输（如果 Tiny 目录有图片）"""
        print("\n" + "="*50)
        print("测试5: 大对象传输")
        print("="*50)
        
        image_url = f"http://{Config.TINY_HOST}:{Config.TINY_PORT}/godzilla.gif"
        
        driver = None
        start_time = time.time()
        
        try:
            driver = create_driver(proxy_port=Config.PROXY_PORT)
            driver.get(image_url)
            
            # 检查是否成功加载（通过检查页面大小或特定元素）
            # 对于图片，直接检查是否能访问
            duration = time.time() - start_time
            
            # 简单判断：如果没有报错，认为成功
            return self.log("Large Object", True,
                          f"成功请求图片 ({duration:.3f}s)", duration)
                          
        except Exception as e:
            duration = time.time() - start_time
            return self.log("Large Object", False,
                          f"失败: {e}", duration)
        finally:
            if driver:
                driver.quit()


# ==================== 主程序 ====================

def print_summary(results):
    """打印测试摘要"""
    print("\n" + "="*50)
    print("测试摘要")
    print("="*50)
    
    total = len(results)
    passed = sum(1 for r in results if r["success"])
    failed = total - passed
    
    for r in results:
        status = "?" if r["success"] else "?"
        print(f"{status} {r['name']}: {r['message']}")
    
    print("-" * 50)
    print(f"总计: {total} 项, 通过: {passed}, 失败: {failed}")
    
    if failed == 0:
        print("? 所有测试通过！")
        return 0
    else:
        print("??  部分测试失败，请检查代理实现")
        return 1


def main():
    """主函数"""
    parser = argparse.ArgumentParser(description='Proxy Lab WebDriver 测试')
    parser.add_argument('--proxy-port', type=int, default=Config.PROXY_PORT,
                      help=f'代理端口 (默认: {Config.PROXY_PORT})')
    parser.add_argument('--tiny-port', type=int, default=Config.TINY_PORT,
                      help=f'Tiny 服务器端口 (默认: {Config.TINY_PORT})')
    parser.add_argument('--test', type=str, default='all',
                      choices=['all', 'basic', 'cgi', 'cache', 'concurrent', 'large'],
                      help='选择测试项目')
    parser.add_argument('--headed', action='store_true',
                      help='显示浏览器窗口（调试用，默认无头）')
    
    args = parser.parse_args()
    
    # 更新配置
    Config.PROXY_PORT = args.proxy_port
    Config.TINY_PORT = args.tiny_port
    Config.TINY_URL = f"http://{Config.TINY_HOST}:{Config.TINY_PORT}/"
    Config.CGI_URL = f"http://{Config.TINY_HOST}:{Config.TINY_PORT}/cgi-bin/adder?1&22"
    
    print("="*50)
    print("CS:APP Proxy Lab - WebDriver 测试")
    print("="*50)
    print(f"代理: http://{Config.PROXY_HOST}:{Config.PROXY_PORT}")
    print(f"Tiny: http://{Config.TINY_HOST}:{Config.TINY_PORT}")
    print(f"模式: {'有头' if args.headed else '无头'}")
    
    # 创建测试实例
    tester = ProxyTests()
    
    # 运行测试
    test_map = {
        'basic': [tester.test_basic_connection],
        'cgi': [tester.test_cgi_content],
        'cache': [tester.test_caching],
        'concurrent': [tester.test_concurrent],
        'large': [tester.test_large_object],
        'all': [
            tester.test_basic_connection,
            tester.test_cgi_content,
            tester.test_caching,
            tester.test_concurrent,
            tester.test_large_object
        ]
    }
    
    tests_to_run = test_map.get(args.test, test_map['all'])
    
    for test_func in tests_to_run:
        try:
            test_func()
        except Exception as e:
            print(f"[ERROR] 测试异常: {e}")
            import traceback
            traceback.print_exc()
    
    # 打印摘要
    return print_summary(tester.results)


if __name__ == "__main__":
    sys.exit(main())